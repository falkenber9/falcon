/*
 * Copyright (c) 2019 Robert Falkenberg.
 *
 * This file is part of FALCON 
 * (see https://github.com/falkenber9/falcon).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */

#include <iostream>
#include <fstream>
#include <signal.h>
#include <unistd.h>

#include "EyeCore.h"
#include "falcon/common/SubframeBuffer.h"
#include "falcon/phy/falcon_rf/rf_imp.h"

#include "falcon/prof/Lifetime.h"

#include "srslte/srslte.h"
// include C-only headers
#ifdef __cplusplus
    extern "C" {
#endif

#include "falcon/phy/falcon_ue/falcon_ue_dl.h"

#ifdef __cplusplus
}
#undef I // Fix complex.h #define I nastiness when using C++
#endif

//#include <complex>


using namespace std;

/* Buffers for PCH reception (not included in DL HARQ) */
const static uint32_t  pch_payload_buffer_sz = 8*1024;  // cf. srslte: srsue/hdr/mac/mac.h

static cell_search_cfg_t cell_detect_config = {
  SRSLTE_DEFAULT_MAX_FRAMES_PBCH,
  SRSLTE_DEFAULT_MAX_FRAMES_PSS,
  SRSLTE_DEFAULT_NOF_VALID_PSS_FRAMES,
  0
};

bool isZero(double value) {
  const double epsilon = 1e-5;
  return (value < epsilon) && (value > -epsilon);
}

bool isEqual(double a, double b, double epsilon) {
    double diff = a - b;
    return (diff < epsilon) && (diff > -epsilon);
}

EyeCore::EyeCore(Args& args) :
  go_exit(false),
  args(args),
  state(DECODE_MIB)
{
  for (int i = 0; i< SRSLTE_MAX_CODEWORDS; i++) {
    pch_payload_buffers[i] = new uint8_t[pch_payload_buffer_sz];
    if (!pch_payload_buffers[i]) {
      cout << "Error allocating bpch_payload_buffers" << endl;
      go_exit = true;
    }
  }
}

EyeCore::~EyeCore() {
  for (int i = 0; i< SRSLTE_MAX_CODEWORDS; i++) {
    delete[] pch_payload_buffers[i];
    pch_payload_buffers[i] = nullptr;
  }
}

bool EyeCore::run() {
  int n, ret;
  int decimate = 1;
  srslte_cell_t cell;
  int64_t sf_cnt;
  srslte_ue_dl_t ue_dl;
  falcon_ue_dl_t falcon_ue_dl;
  srslte_ue_sync_t ue_sync;
  srslte_ue_mib_t ue_mib;
#ifndef DISABLE_RF
  srslte_rf_t rf;
#endif

  uint8_t bch_payload[SRSLTE_BCH_PAYLOAD_LEN];
  int32_t sfn_offset = 0;
  float cfo = 0;
  //FILE *fid;
  //uint16_t rnti_tmp;

  SubframeBuffer sfb(args.rf_nof_rx_ant);
  uint32_t sfn = 0; // system frame number

  if(args.cpu_affinity > -1) {
    cpu_set_t cpuset;
    pthread_t thread;

    thread = pthread_self();
    for(int i = 0; i < 8;i++){
      if(((args.cpu_affinity >> i) & 0x01) == 1){
        cout << "Setting cni_capture_sync with affinity to core " << i << endl;
        CPU_SET(static_cast<size_t>(i), &cpuset);
      }
      if(pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset)){
        cout << "Error setting main thread affinity to " << args.cpu_affinity << endl;
        return true;
      }
    }
  }

#ifndef DISABLE_RF
  if (args.input_file_name == "") {
    cout << "Opening RF device with " << args.rf_nof_rx_ant <<
                " RX antennas..." << endl;
    char rfArgsCStr[1024];  /* WTF! srslte_rf_open_multi takes char*, not const char* ! */
    strncpy(rfArgsCStr, args.rf_args.c_str(), 1024);
    if (srslte_rf_open_multi(&rf, rfArgsCStr, args.rf_nof_rx_ant)) {
      cout <<  "Error opening rf" << endl;
      return true;
    }
    /* Set receiver gain */
    if (args.rf_gain > 0) {
      srslte_rf_set_rx_gain(&rf, args.rf_gain);
    }
    else {
      cout << "Starting AGC thread..." << endl;
      if (srslte_rf_start_gain_thread(&rf, false)) {
        cout << "Error starting AGC thread" << endl;
        return true;
      }
      srslte_rf_set_rx_gain(&rf, srslte_rf_get_rx_gain(&rf));
      cell_detect_config.init_agc = static_cast<float>(srslte_rf_get_rx_gain(&rf));
    }

    srslte_rf_set_master_clock_rate(&rf, 30.72e6);

    /* set receiver frequency */
    double rf_freq = args.rf_freq;
    cout << "Tunning receiver to " << rf_freq << " Hz" << endl;
    srslte_rf_set_rx_freq(&rf, rf_freq);
    srslte_rf_rx_wait_lo_locked(&rf);

    uint32_t ntrial=0;
    uint32_t max_trial = 3;
    do {
      ret = rf_search_and_decode_mib(&rf, args.rf_nof_rx_ant, &cell_detect_config, args.force_N_id_2, &cell, &cfo);
      if (ret < 0) {
        cout << "Error searching for cell" << endl;
        go_exit = true;
      } else if (ret == 0 && !go_exit) {
        cout << "Cell not found after " << ntrial++ << " trials" << endl;
      }
      if (ntrial >= max_trial) go_exit = true;
    } while (ret == 0 && !go_exit);

    srslte_rf_stop_rx_stream(&rf);
    //srslte_rf_flush_buffer(&rf);

    if (go_exit) {
      srslte_rf_kill_gain_thread(&rf);
      srslte_rf_close(&rf);
      return true;
    }

    /* set sampling frequency */
    int srate = srslte_sampling_freq_hz(cell.nof_prb);
    if (srate != -1) {
      if (srate < 10e6) {
        srslte_rf_set_master_clock_rate(&rf, 4*srate);
      } else {
        srslte_rf_set_master_clock_rate(&rf, srate);
      }
      cout << "Setting sampling rate " << (srate)/1000000 << " MHz" << endl;
      double srate_rf = srslte_rf_set_rx_srate(&rf, static_cast<double>(srate));
      if (!isEqual(srate_rf, srate, 1.0)) {
        cout << "Could not set sampling rate" << endl;
        return true;
      }
    } else {
      cout << "Invalid number of PRB " << cell.nof_prb << endl;
      return true;
    }

    cout << "Stopping RF and flushing buffer..." << endl;
  }
#endif

  /* If reading from file, go straight to PDSCH decoding. Otherwise, decode MIB first */
  if (args.input_file_name != "") {
    /* preset cell configuration */
    cell.id = args.file_cell_id;
    cell.cp = SRSLTE_CP_NORM;
    cell.phich_length = SRSLTE_PHICH_NORM;
    cell.phich_resources = SRSLTE_PHICH_R_1;
    cell.nof_ports = args.file_nof_ports;
    cell.nof_prb = args.file_nof_prb;

    char* tmp_filename = new char[args.input_file_name.length()+1]; /* WTF! srslte_ue_sync_init_file takes char*, not const char* ! */
    strncpy(tmp_filename, args.input_file_name.c_str(), args.input_file_name.length());
    tmp_filename[args.input_file_name.length()] = 0;  // 0 termination for safety
    int ret = srslte_ue_sync_init_file(&ue_sync,
                                 args.file_nof_prb,
                                 tmp_filename,
                                 args.file_offset_time,
                                 static_cast<float>(args.file_offset_freq));
    delete[] tmp_filename;
    tmp_filename = nullptr;
    if (ret) {
      cout << "Error initiating ue_sync" << endl;
      return true;
    }
  }
  else {
#ifndef DISABLE_RF
    if(args.decimate) {
      if(args.decimate > 4 || args.decimate < 0) {
        cout << "Invalid decimation factor, setting to 1" << endl;
      }
      else {
        decimate = args.decimate;
        //ue_sync.decimate = prog_args.decimate;
      }
    }
    if (srslte_ue_sync_init_multi_decim(&ue_sync,
                                        cell.nof_prb,
                                        cell.id==1000,
                                        falcon_rf_recv_wrapper,
                                        args.rf_nof_rx_ant,
                                        static_cast<void*>(&rf), decimate)) {
      cout << "Error initiating ue_sync" << endl;
      return true;
    }
    if (srslte_ue_sync_set_cell(&ue_sync, cell)) {
      cout << "Error initiating ue_sync" << endl;
      return true;
    }
#endif
  }

  if (srslte_ue_mib_init(&ue_mib, sfb.sf_buffer, cell.nof_prb)) {
    cout << "Error initaiting UE MIB decoder" << endl;
    return true;
  }
  if (srslte_ue_mib_set_cell(&ue_mib, cell)) {
    cout << "Error initaiting UE MIB decoder" << endl;
    return true;
  }

  if (falcon_ue_dl_init(&falcon_ue_dl, &ue_dl, sfb.sf_buffer, cell.nof_prb, args.rf_nof_rx_ant, args.dci_file_name.c_str(), args.stats_file_name.c_str())) {
    //if (srslte_ue_dl_init(&ue_dl, sf_buffer, cell.nof_prb, args.rf_nof_rx_ant)) {
    cout << "Error initiating UE downlink processing module" << endl;
    return true;
  }
  if (srslte_ue_dl_set_cell(&ue_dl, cell)) {
    cout << "Error initiating UE downlink processing module" << endl;
    return true;
  }

  // Disable CP based CFO estimation during find
  ue_sync.cfo_current_value = cfo/15000;
  ue_sync.cfo_is_copied = true;
  ue_sync.cfo_correct_enable_find = true;
  srslte_sync_set_cfo_cp_enable(&ue_sync.sfind, false, 0);


  srslte_chest_dl_cfo_estimate_enable(&ue_dl.chest, false, 1023); // args->enable_cfo_ref = false;
  srslte_chest_dl_average_subframe(&ue_dl.chest, false);          // args->average_subframe = false;

  /* Configure downlink receiver for the SI-RNTI since will be the only one we'll use */
  srslte_ue_dl_set_rnti(&ue_dl, SRSLTE_SIRNTI);

  /* Initialize subframe counter */
  sf_cnt = 0;

//  if (!args.disable_plots) {
//    init_plots(cell);
//    bzero(rb_up, 1024*sizeof(float));
//    bzero(rb_dw, 1024*sizeof(float));
//    bzero(bw_up, 1024*sizeof(float));
//    bzero(bw_dw, 1024*sizeof(float));
//  }

#ifndef DISABLE_RF
  if (args.input_file_name == "") {
    srslte_rf_start_rx_stream(&rf, false);
  }
#endif

#ifndef DISABLE_RF
  if (args.rf_gain < 0 && args.input_file_name == "") {
    srslte_rf_info_t *rf_info = srslte_rf_get_info(&rf);
    srslte_ue_sync_start_agc(&ue_sync,
                             falcon_rf_set_rx_gain_th_wrapper,
                             rf_info->min_rx_gain,
                             rf_info->max_rx_gain,
                             static_cast<double>(cell_detect_config.init_agc));
  }
#endif

  ue_sync.cfo_correct_enable_track = !args.disable_cfo;

  falcon_ue_dl.q = &ue_dl;
//  if (args.rnti_list_file != NULL) { // loading RNTI list
//    fid = fopen(args.rnti_list_file,"r");
//    if (fid>0) {
//      for (int i=0;i<65536;i++) {
//        if (fscanf(fid,"%hu",&rnti_tmp) != EOF) {
//          //		  if (rnti_tmp) printf("rnti %d val %d\n", i, rnti_tmp);
//          srslte_ue_dl_reset_rnti_user_to(&falcon_ue_dl, i, rnti_tmp); // check this
//          //printf("is rnti in the list? %d\n",rnti_in_list(&ue_dl, rnti_tmp));
//        }
//      }
//      fclose(fid);
//    }
//  }

  srslte_pbch_decode_reset(&ue_mib.pbch);

  cout << "Entering main loop..." << endl;
  /* Main loop */
  while (!go_exit && (sf_cnt < args.nof_subframes || args.nof_subframes == 0)) {

    bool acks [SRSLTE_MAX_CODEWORDS] = {false};
    ret = srslte_ue_sync_zerocopy_multi(&ue_sync, sfb.sf_buffer);
    if (ret < 0) {
      cout << "Error calling srslte_ue_sync_work()" << endl;
    }
    /* TODO: Add return value 7 for controlled shutdown after reading whole file in file mode */
    if (ret == 7) {
//      if (!args.disable_plots) {
//        plot_sf_idx = sfn % 1024;
//        last_good = sfn;
//        plot_track = true;
//        sem_post(&plot_sem);
//      }
      go_exit = true;
      break;
    }

#ifdef CORRECT_SAMPLE_OFFSET
    float sample_offset = (float) srslte_ue_sync_get_last_sample_offset(&ue_sync)+srslte_ue_sync_get_sfo(&ue_sync)/1000;
    srslte_ue_dl_set_sample_offset(&ue_dl, sample_offset);
#endif

    /* srslte_ue_sync_zerocopy_multi returns 1 if successfully read 1 aligned subframe */
    if (ret == 1) {
      switch (state) {
        case DECODE_MIB:
          if (srslte_ue_sync_get_sfidx(&ue_sync) == 0) {
            n = srslte_ue_mib_decode(&ue_mib, bch_payload, nullptr, &sfn_offset);
            if (n < 0) {
              cout << "Error decoding UE MIB" << endl;
              go_exit = true;
            } else if (n == SRSLTE_UE_MIB_FOUND) {
              srslte_pbch_mib_unpack(bch_payload, &cell, &sfn);
              //srslte_cell_fprint(stdout, &cell, sfn);
              cout << "Decoded MIB. SFN: " << sfn <<
                      ", offset " << sfn_offset << endl;
              sfn = (sfn + static_cast<uint32_t>(sfn_offset)) % 1024;
              state = DECODE_PDSCH;
              if(falcon_ue_dl.rnti_manager != nullptr) {
                rnti_manager_free(falcon_ue_dl.rnti_manager);
                falcon_ue_dl.rnti_manager = nullptr;
              }
              falcon_ue_dl.rnti_manager = rnti_manager_create(nof_falcon_ue_all_formats, RNTI_PER_SUBFRAME);
              // setup rnti manager
              int idx;
              // add format1A evergreens
              idx = falcon_dci_index_of_format_in_list(SRSLTE_DCI_FORMAT1A, falcon_ue_all_formats, nof_falcon_ue_all_formats);
              if(idx > -1) {
                rnti_manager_add_evergreen(falcon_ue_dl.rnti_manager, SRSLTE_RARNTI_START, SRSLTE_RARNTI_END, static_cast<uint32_t>(idx));
                rnti_manager_add_evergreen(falcon_ue_dl.rnti_manager, SRSLTE_PRNTI, SRSLTE_SIRNTI, static_cast<uint32_t>(idx));
              }
              // add format1C evergreens
              idx = falcon_dci_index_of_format_in_list(SRSLTE_DCI_FORMAT1C, falcon_ue_all_formats, nof_falcon_ue_all_formats);
              if(idx > -1) {
                rnti_manager_add_evergreen(falcon_ue_dl.rnti_manager, SRSLTE_RARNTI_START, SRSLTE_RARNTI_END, static_cast<uint32_t>(idx));
                rnti_manager_add_evergreen(falcon_ue_dl.rnti_manager, SRSLTE_PRNTI, SRSLTE_SIRNTI, static_cast<uint32_t>(idx));
              }
              // add forbidden rnti values to rnti manager
              for(uint32_t f=0; f<nof_falcon_ue_all_formats; f++) {
                  //disallow RNTI=0 for all formats
                  rnti_manager_add_forbidden(falcon_ue_dl.rnti_manager, 0x0, 0x0, f);
              }

            }
          }
          break;
        case DECODE_PDSCH:
          {
            //PrintLifetime lt("###>> Subframe took: ");
            srslte_ue_dl_get_control_cc_hist(&falcon_ue_dl, srslte_ue_sync_get_sfidx(&ue_sync), sfn);
          }
          if (ue_dl.current_rnti != 0xffff) {
            //n = srslte_ue_dl_decode_broad(&ue_dl, &sf_buffer[args.time_offset], data, srslte_ue_sync_get_sfidx(&ue_sync), ue_dl.current_rnti);
            if(cell.nof_ports == 1) {
              /* Transmission mode 1 */
              n = srslte_ue_dl_decode(&ue_dl, pch_payload_buffers, 0, sfn * 10 + srslte_ue_sync_get_sfidx(&ue_sync), acks);
            }
            else {
              /* Transmission mode 2 */
              n = srslte_ue_dl_decode(&ue_dl, pch_payload_buffers, 1, sfn * 10 + srslte_ue_sync_get_sfidx(&ue_sync), acks);
            }
            uint8_t* pch_payload = pch_payload_buffers[0];
            uint16_t t_rnti;

            switch(n) {
              case 0:
                //        			printf("No decode\n");
                break;
              case 40:
                t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-2]) + pch_payload[(n/8)-1];
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
                rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);
                //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-2] + pch_payload[(n/8)-1]);
                //        			for (int k=0; k<(n/8); k++) {
                //        				printf("%02x ",pch_payload[k]);
                //        			}
                //        			printf("\n");
                break;
              case 56:
                t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-2]) + pch_payload[(n/8)-1];
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
                rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);
                //        			printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-2] + pch_payload[(n/8)-1]);
                //        			for (int k=0; k<(n/8); k++) {
                //        				printf("%02x ",pch_payload[k]);
                //        			}
                //        			printf("\n");
                break;
              case 72:
                t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-4]) + pch_payload[(n/8)-3];
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
                rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);
                //        			printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-4] + pch_payload[(n/8)-3]);
                //        			for (int k=0; k<(n/8); k++) {
                //        				printf("%02x ",pch_payload[k]);
                //        			}
                //        			printf("\n");
                break;
              case 120:
                t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-3]) + pch_payload[(n/8)-2];
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
                rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);

                t_rnti = 256*static_cast<uint16_t>(pch_payload[(n/8)-9]) + pch_payload[(n/8)-8];
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, t_rnti);
                rnti_manager_activate_and_refresh(falcon_ue_dl.rnti_manager, t_rnti, 0, ActivationReason::RM_ACT_RAR);
                //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-9] + pch_payload[(n/8)-8]);
                //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-3] + pch_payload[(n/8)-2]);
                //					for (int k=0; k<(n/8); k++) {
                //						printf("%02x ",pch_payload[k]);
                //					}
                //					printf("\n");
                break;
              default:
                //        			fprintf(stderr,"\n");
                //					for (int k=0; k<(n/8); k++) {
                //						fprintf(stderr,"%02x ",pch_payload[k]);
                //					}
                //					fprintf(stderr,"\n");
                break;
            }
          }
          break;
      }

      if (srslte_ue_sync_get_sfidx(&ue_sync) == 9) {
        sfn++;
      }

#ifdef BLOCK_SUBSCRIBE
      if (!args.disable_plots) {
        if (state == DECODE_PDSCH) {
          //plot_sf_idx = sfn % 1024;
          //last_good = sfn;
          //plot_track = true;
          float* current_colored_rb_map_up = falcon_ue_dl.colored_rb_map_up_last;
          float* current_colored_rb_map_dw = falcon_ue_dl.colored_rb_map_dw_last;

          //ScanLine uplinkScanLine(current_colored_rb_map_up, ue_dl.cell.nof_prb);
          //uplinkAllocProvider.pushToSubscribers(&uplinkScanLine);

          //ScanLine downlinkScanLine(current_colored_rb_map_dw, ue_dl.cell.nof_prb);
          //downlinkAllocProvider.pushToSubscribers(&downlinkScanLine);

          // Spectrum - compute absolute values
          float tmp_plot_wf[2048*110*15];
          bzero(tmp_plot_wf,12*ue_dl.cell.nof_prb*sizeof(float));
          for (int j = 0; j < 14; j++) {
            for (int i = 0; i < 12*ue_dl.cell.nof_prb; i++) { // 12*
              //tmp_plot_wf[i] += 20 * log10f(cabsf(ue_dl.sf_symbols[i+j*(12*ue_dl.cell.nof_prb)]))/14;
              tmp_plot_wf[i] += 20 * log10f(std::abs(static_cast<std::complex<float>>(ue_dl.sf_symbols[i+j*(12*ue_dl.cell.nof_prb)])))/14;
              // if(ue_dl == NULL) tmp_plot_wf[i] = 0;
              // printf(": %f",ue_dl.sf_symbols[i+j*12*ue_dl.cell.nof_prb]);

            }
          }

          // Spectrum - average signals to full PRBs
          float tmp_linebuffer[110];
          float tmp_buff = 0;

          for(uint32_t i = 0; i < ue_dl.cell.nof_prb; i++){
            for(uint32_t j = 0; j < 12; j++){
              if((i*12)+j >= 600) break;
              tmp_buff += tmp_plot_wf[(i*12)+j];
            }
            tmp_linebuffer[i] = tmp_buff * 128;
            tmp_buff = 0;
          }

          //ScanLine spectrumScanLine(tmp_linebuffer, ue_dl.cell.nof_prb);
          //downlinkSpectrumProvider.pushToSubscribers(&spectrumScanLine);

          // Histogram
          uint32_t hist_int[65536];
          rnti_manager_get_histogram_summary(falcon_ue_dl.rnti_manager, hist_int);

          //histogramProvider.pushToSubscribers(hist_int);

          //call_function(decoderthread,falcon_ue_dl.colored_rb_map_up_last, falcon_ue_dl.colored_rb_map_dw_last,tmp_linebuffer,hist_int);
          //sem_post(&plot_sem);
        }
      }
#endif

      if (sfn % 10 == 0 && srslte_ue_sync_get_sfidx(&ue_sync) == 9) {
        if(SRSLTE_VERBOSE_ISINFO()) {
          rnti_manager_print_active_set(falcon_ue_dl.rnti_manager);
        }
      }
    }
    else if (ret == 0) {
      cout << "Finding PSS... Peak: " << srslte_sync_get_peak_value(&ue_sync.sfind) <<
              ", FrameCnt: " << ue_sync.frame_total_cnt <<
              " State: " << ue_sync.state << endl;
    }
    // Some delay when playing
    if (args.input_file_name != "" && !args.disable_plots) {
      usleep(1000);
    }
    sf_cnt++;
  } // Main loop

  rnti_manager_print_active_set(falcon_ue_dl.rnti_manager);

  srslte_ue_dl_stats_print(&falcon_ue_dl, falcon_ue_dl.stats_file);

  rnti_manager_free(falcon_ue_dl.rnti_manager);
  falcon_ue_dl_free(&falcon_ue_dl);
  srslte_ue_sync_free(&ue_sync);
  srslte_ue_mib_free(&ue_mib);

#ifndef DISABLE_RF
  if (args.input_file_name == "") {
    srslte_rf_close(&rf);
  }
#endif

  return true;
}

void EyeCore::handleSignal() {
  cout << "EyeCore: Exiting..." << endl;
  go_exit = true;
}
