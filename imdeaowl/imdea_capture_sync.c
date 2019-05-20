/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2015 Software Radio Systems Limited
 * Copyright 2016 IMDEA Networks Institute
 *
 * \section LICENSE
 *
 * This file is part of OWL, which extends the srsLTE library.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include "srslte/phy/common/phy_common.h"

#include "srslte/common/crash_handler.h"
#include "srslte/srslte.h"

#define ENABLE_AGC_DEFAULT

#ifndef DISABLE_RF
#include "srslte/phy/rf/rf.h"
#include "srslte/phy/rf/rf_utils.h"

static cell_search_cfg_t cell_detect_config = {
  SRSLTE_DEFAULT_MAX_FRAMES_PBCH,
  SRSLTE_DEFAULT_MAX_FRAMES_PSS,
  SRSLTE_DEFAULT_NOF_VALID_PSS_FRAMES,
  0
};

#else
#warning Compiling pdsch_ue with no RF support
#endif

bool isEqual(double a, double b, double epsilon) {
    double diff = a - b;
    return (diff < epsilon) && (diff > -epsilon);
}

//#define STDOUT_COMPACT

#ifndef DISABLE_GRAPHICS
#include "srsgui/srsgui.h"
void init_plots(void);
static pthread_t plot_thread;
static sem_t plot_sem;
static uint32_t plot_sf_idx=0;
static bool plot_track = true;
#endif

#define PRINT_CHANGE_SCHEDULIGN

//#define CORRECT_SAMPLE_OFFSET

/**********************************************************************
 *  Program arguments processing
 ***********************************************************************/
typedef struct {
  int nof_subframes;
  int cpu_affinity;
  bool disable_cfo;
  int force_N_id_2;
  char *output_file_name;
  uint32_t nof_prb;
  char *rf_args;
  uint32_t rf_nof_rx_ant;
  double rf_freq;
  float rf_gain;
  int decimate;
}prog_args_t;

void args_default(prog_args_t *args) {
  args->nof_subframes = -1;
  args->force_N_id_2 = -1; // Pick the best
  args->output_file_name = NULL;
  args->disable_cfo = false;
  args->nof_prb = 25;
  args->rf_args = "";
  args->rf_freq = -1.0;
  args->rf_nof_rx_ant = 1;
#ifdef ENABLE_AGC_DEFAULT
  args->rf_gain = -1.0;
#else
  args->rf_gain = 50.0;
#endif
  args->decimate = 0;
  args->cpu_affinity = -1;
}

void usage(prog_args_t *args, char *prog) {
  printf("Usage: %s [aACfglnopvyZ] -f rx_frequency_hz -o output_file\n", prog);
  printf("\t-a RF args [Default %s]\n", args->rf_args);
  printf("\t-A Number of RX antennas [Default %d]\n", args->rf_nof_rx_ant);
  printf("\t-C Disable CFO correction [Default %s]\n", args->disable_cfo?"Disabled":"Enabled");
#ifdef ENABLE_AGC_DEFAULT
  printf("\t-g RF fix RX gain [Default AGC]\n");
#else
  printf("\t-g Set RX gain [Default %.1f dB]\n", args->rf_gain);
#endif
  printf("\t-l Force N_id_2 [Default best]\n");
  printf("\t-n nof_subframes [Default %d]\n", args->nof_subframes);
  printf("\t-p nof_prb [Default %d]\n", args->nof_prb);
  printf("\t-v [set srslte_verbose to debug, default none]\n");
  printf("\t-y set the cpu affinity mask [Default %d] \n  ",args->cpu_affinity);
  printf("\t-Z set decimation value [Default %d] \n  ",args->decimate);
}

void parse_args(prog_args_t *args, int argc, char **argv) {
  int opt;
  args_default(args);
  while ((opt = getopt(argc, argv, "aACfglnopvy")) != -1) {
    switch (opt) {
      case 'p':
        args->nof_prb = atoi(argv[optind]);
        break;
      case 'o':
        args->output_file_name = argv[optind];
        break;
      case 'a':
        args->rf_args = argv[optind];
        break;
      case 'A':
        args->rf_nof_rx_ant = atoi(argv[optind]);
        break;
      case 'g':
        args->rf_gain = atof(argv[optind]);
        break;
      case 'C':
        args->disable_cfo = true;
        break;
      case 'f':
        args->rf_freq = strtod(argv[optind], NULL);
        break;
      case 'n':
        args->nof_subframes = atoi(argv[optind]);
        break;
      case 'l':
        args->force_N_id_2 = atoi(argv[optind]);
        break;
      case 'v':
        srslte_verbose++;
        break;
      case 'Z':
        args->decimate = atoi(argv[optind]);
        break;
      case 'y':
        args->cpu_affinity = atoi(argv[optind]);
        break;
      default:
        usage(args, argv[0]);
        exit(-1);
    }
  }
  if (args->rf_freq < 0 || args->output_file_name == NULL) {
    usage(args, argv[0]);
    exit(-1);
  }
}
/**********************************************************************/

/* TODO: Do something with the output data */
static uint8_t data[20000];

static bool go_exit = false;
void sig_int_handler(int signo)
{
  printf("SIGINT received. Exiting...\n");
  if (signo == SIGINT) {
    go_exit = true;
  }
}

static cf_t *sf_buffer[2] = {NULL, NULL};

int srslte_rf_recv_wrapper(void *h, cf_t *data[SRSLTE_MAX_PORTS], uint32_t nsamples, srslte_timestamp_t *t) {
  DEBUG(" ----  Receive %d samples  ---- \n", nsamples);
  void *ptr[SRSLTE_MAX_PORTS];
  for (int i=0;i<SRSLTE_MAX_PORTS;i++) {
    ptr[i] = data[i];
  }
  return srslte_rf_recv_with_time_multi(h, ptr, nsamples, true, NULL, NULL);
}

double srslte_rf_set_rx_gain_th_wrapper_(void *h, double f) {
  return srslte_rf_set_rx_gain_th((srslte_rf_t*) h, f);
}

extern float mean_exec_time;

static enum receiver_state { DECODE_MIB, } state;

static srslte_ue_dl_t ue_dl;
static srslte_ue_sync_t ue_sync;
static prog_args_t prog_args;

static uint32_t sfn = 0; // system frame number

int main(int argc, char **argv) {
  int n, ret;
  int decimate = 1;
  cf_t *sf_buffer[SRSLTE_MAX_PORTS] = {NULL, NULL};
  srslte_cell_t cell;
  int64_t sf_cnt, sf_guard;
  srslte_ue_mib_t ue_mib;
  srslte_rf_t rf;
  srslte_filesink_t sink;
  uint8_t bch_payload[SRSLTE_BCH_PAYLOAD_LEN];
  int sfn_offset;
  bool fstart = 0;
  float cfo = 0;

  srslte_debug_handle_crash(argc, argv);

  parse_args(&prog_args, argc, argv);

  // TODO multiply the output files
  srslte_filesink_init(&sink, prog_args.output_file_name, SRSLTE_COMPLEX_FLOAT_BIN);

  if(prog_args.cpu_affinity > -1) {

    cpu_set_t cpuset;
    pthread_t thread;

    thread = pthread_self();
    for(int i = 0; i < 8;i++){
      if(((prog_args.cpu_affinity >> i) & 0x01) == 1){
        printf("Setting pdsch_ue with affinity to core %d\n", i);
        CPU_SET((size_t) i , &cpuset);
      }
      if(pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset)){
        fprintf(stderr, "Error setting main thread affinity to %d \n", prog_args.cpu_affinity);
        exit(-1);
      }
    }
  }
  printf("Opening RF device with %d RX antennas...\n", prog_args.rf_nof_rx_ant);
  if (srslte_rf_open_multi(&rf, prog_args.rf_args, prog_args.rf_nof_rx_ant)) {
    fprintf(stderr, "Error opening rf\n");
    exit(-1);
  }
  /* Set receiver gain */
  if (prog_args.rf_gain > 0) {
    srslte_rf_set_rx_gain(&rf, prog_args.rf_gain);
  } else {
    printf("Starting AGC thread...\n");
    if (srslte_rf_start_gain_thread(&rf, false)) {
      fprintf(stderr, "Error opening rf\n");
      exit(-1);
    }
    srslte_rf_set_rx_gain(&rf, srslte_rf_get_rx_gain(&rf));
    cell_detect_config.init_agc = srslte_rf_get_rx_gain(&rf);
  }

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigprocmask(SIG_UNBLOCK, &sigset, NULL);
  signal(SIGINT, sig_int_handler);

  srslte_rf_set_master_clock_rate(&rf, 30.72e6);

  /* set receiver frequency */
  printf("Tunning receiver to %.3f MHz\n", prog_args.rf_freq/1000000);
  srslte_rf_set_rx_freq(&rf, prog_args.rf_freq);
  srslte_rf_rx_wait_lo_locked(&rf);

  uint32_t ntrial=0;
  uint32_t max_trial=3;
  do {
    ret = rf_search_and_decode_mib(&rf, prog_args.rf_nof_rx_ant, &cell_detect_config, prog_args.force_N_id_2, &cell, &cfo);
    if (ret < 0) {
      fprintf(stderr, "Error searching for cell\n");
      exit(-1);
    } else if (ret == 0 && !go_exit) {
      printf("Cell not found after %d trials. Trying again (Press Ctrl+C to exit)\n", ntrial++);
    }
    if (ntrial >= max_trial) go_exit = true;
  } while (ret == 0 && !go_exit);


  if (go_exit) {
    srslte_rf_close(&rf);
    exit(0);
  }

#ifdef DISABLED__
  srslte_rf_stop_rx_stream(&rf);
  srslte_rf_flush_buffer(&rf);
#endif

  /* set sampling frequency */
  int srate = srslte_sampling_freq_hz(cell.nof_prb);
  if (srate != -1) {
    if (srate < 10e6) {
      srslte_rf_set_master_clock_rate(&rf, 4*srate);
    } else {
      srslte_rf_set_master_clock_rate(&rf, srate);
    }
    printf("Setting sampling rate %.2f MHz\n", (float) srate/1000000);
    float srate_rf = srslte_rf_set_rx_srate(&rf, (double) srate);
    if (!isEqual(srate_rf, srate, 1.0)) {
      fprintf(stderr, "Could not set sampling rate\n");
      exit(-1);
    }
  } else {
    fprintf(stderr, "Invalid number of PRB %d\n", cell.nof_prb);
    exit(-1);
  }

  INFO("Stopping RF and flushing buffer...\r");
#ifdef DISABLED__
  srslte_rf_stop_rx_stream(&rf);
  srslte_rf_flush_buffer(&rf);
#endif

  if(prog_args.decimate) {
    if(prog_args.decimate > 4 || prog_args.decimate < 0) {
      printf("Invalid decimation factor, setting to 1 \n");
    }
    else {
      decimate = prog_args.decimate;
      //ue_sync.decimate = prog_args.decimate;
    }
  }
  if (srslte_ue_sync_init_multi_decim(&ue_sync,
                                      cell.nof_prb,
                                      cell.id==1000,
                                      srslte_rf_recv_wrapper,
                                      prog_args.rf_nof_rx_ant,
                                      (void*) &rf, decimate)) {
    fprintf(stderr, "Error initiating ue_sync\n");
    exit(-1);
  }

  if (srslte_ue_sync_set_cell(&ue_sync, cell)) {
    fprintf(stderr, "Error initiating ue_sync\n");
    return -1;
  }

  for (uint32_t i=0;i<prog_args.rf_nof_rx_ant;i++) {
    sf_buffer[i] = srslte_vec_malloc(3*sizeof(cf_t)*SRSLTE_SF_LEN_PRB(cell.nof_prb));
  }

  if (srslte_ue_dl_init(&ue_dl, sf_buffer, cell.nof_prb, prog_args.rf_nof_rx_ant)) {
    fprintf(stderr, "Error initiating UE downlink processing module\n");
    return -1;
  }

  if (srslte_ue_dl_set_cell(&ue_dl, cell)) {
    fprintf(stderr, "Error initiating UE downlink processing module\n");
    return -1;
  }

  if (srslte_ue_mib_init(&ue_mib, sf_buffer, cell.nof_prb)) {
    fprintf(stderr, "Error initaiting UE MIB decoder\n");
    return -1;
  }
  if (srslte_ue_mib_set_cell(&ue_mib, cell)) {
    fprintf(stderr, "Error initaiting UE MIB decoder\n");
    return -1;
  }

  // Disable CP based CFO estimation during find
  ue_sync.cfo_current_value = cfo/15000;
  ue_sync.cfo_is_copied = true;
  ue_sync.cfo_correct_enable_find = true;
  srslte_sync_set_cfo_cp_enable(&ue_sync.sfind, false, 0);


  srslte_chest_dl_cfo_estimate_enable(&ue_dl.chest, false, 1023); // args->enable_cfo_ref = false;
  srslte_chest_dl_average_subframe(&ue_dl.chest, false);          // args->average_subframe = false;

  /* Initialize subframe counter */
  sf_cnt = 0;
  sf_guard = 0;

  srslte_rf_start_rx_stream(&rf, false);

  if (prog_args.rf_gain < 0) {
    srslte_rf_info_t *rf_info = srslte_rf_get_info(&rf);
    srslte_ue_sync_start_agc(&ue_sync,
                             srslte_rf_set_rx_gain_th_wrapper_,
                             rf_info->min_rx_gain,
                             rf_info->max_rx_gain,
                             cell_detect_config.init_agc);
  }

#ifdef PRINT_CHANGE_SCHEDULIGN
  srslte_ra_dl_dci_t old_dl_dci;
  bzero(&old_dl_dci, sizeof(srslte_ra_dl_dci_t));
#endif

  ue_sync.cfo_correct_enable_track = !prog_args.disable_cfo;

  //	// Set initial CFO for ue_sync
  //	srslte_ue_sync_set_cfo(&ue_sync, cfo);

  srslte_pbch_decode_reset(&ue_mib.pbch);

  INFO("\nEntering main loop...\n\n");
  /* Main loop */
  while (!go_exit && (sf_cnt < prog_args.nof_subframes || prog_args.nof_subframes == -1)) {

    ret = srslte_ue_sync_zerocopy_multi(&ue_sync, sf_buffer);
    if (ret < 0) {
      fprintf(stderr, "Error calling srslte_ue_sync_work()\n");
    }

#ifdef CORRECT_SAMPLE_OFFSET
    float sample_offset = (float) srslte_ue_sync_get_last_sample_offset(&ue_sync)+srslte_ue_sync_get_sfo(&ue_sync)/1000;
    srslte_ue_dl_set_sample_offset(&ue_dl, sample_offset);
#endif

    /* srslte_ue_sync_zerocopy_multi returns 1 if successfully read 1 aligned subframe */
    if (ret == 1) {
      uint32_t sfidx = srslte_ue_sync_get_sfidx(&ue_sync);

      switch (state) {
        case DECODE_MIB:
          if (sfidx == 0) {
            //            srslte_pbch_decode_reset(&ue_mib.pbch);
            n = srslte_ue_mib_decode(&ue_mib, bch_payload, NULL, &sfn_offset);
            if (n < 0) {
              fprintf(stderr, "Error decoding UE MIB\n");
              exit(-1);
            } else if (n == SRSLTE_UE_MIB_FOUND) {
              srslte_pbch_mib_unpack(bch_payload, &cell, &sfn);
              //srslte_cell_fprint(stdout, &cell, sfn);
              fprintf(stdout,"Decoded MIB. SFN: %d, offset: %d\n", sfn, sfn_offset);
              //exit(0);
              if (!fstart) {
                fprintf(stderr,"*************************\n"
                               "*************************\n"
                               "Recording started: %d\n"
                               "*************************\n"
                               "*************************\n", sfn);
                sfn = (sfn + sfn_offset)%1024;
                fstart = 1;
              }
            } else {
              fprintf(stdout,"MIB not decoded. SFN: %d, offset: %d\n", sfn, sfn_offset);
            }
          }
          break;
      }
      if (srslte_ue_sync_get_sfidx(&ue_sync) == 9) {
        sfn++;
        //      if (sfn == 1024) {
        //        sfn = 0;
        //      }
      }
      // TODO multiple files
      if (fstart) srslte_filesink_write(&sink, sf_buffer[0], SRSLTE_SF_LEN_PRB(cell.nof_prb));
    }
    else if (ret == 0) {
      if (fstart) {
        fprintf(stderr,"Sync loss at %d\n", sfn);
        go_exit = true;
      }
      else {
        fprintf(stderr,"Finding PSS... Peak: %8.1f, FrameCnt: %d, State: %d\n",
                srslte_sync_get_peak_value(&ue_sync.sfind),
                ue_sync.frame_total_cnt, ue_sync.state);
      }
    }

    if (fstart) sf_cnt++;
    sf_guard++;
    //    if (sf_guard > nof_subframes + 10000) {
    //      fprintf(stderr,"watchdog exit\n");
    //	  go_exit = true;
    //    }
  } // Main loop

  srslte_ue_dl_free(&ue_dl);
  srslte_ue_sync_free(&ue_sync);
  for (uint32_t i = 0; i < prog_args.rf_nof_rx_ant; i++) {
    if (sf_buffer[i]) {
      free(sf_buffer[i]);
    }
  }

  srslte_ue_mib_free(&ue_mib);
  srslte_rf_close(&rf);
  //  printf("\nBye\n");
  if (go_exit) {
    exit(-1);
  } else {
    exit(0);
  }
}

