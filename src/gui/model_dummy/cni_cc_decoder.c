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
#include "falcon/definitions.h"

//MO HACKS:

#include "cni_cc_decoder.h"


#include "falcon/phy/falcon_phch/falcon_dci.h"


//float uplink_data[200];     //200 to be safe
//float downlink_data[200];
void *decoderthread;
volatile prog_args_t prog_args;


//#MO_HACKS

#include "srslte/common/crash_handler.h"
#include "srslte/srslte.h"
#include "falcon/phy/falcon_ue/falcon_ue_dl.h"

#define PLOT_REFRESH_SFN 1 //0 //1, 10
#define ENABLE_AGC_DEFAULT
//#define CORRECT_SAMPLE_OFFSET

#ifdef ENABLE_GUI
#include "srsgui/srsgui.h"
#endif

volatile bool go_exit = false;

float tmp_plot_wf[2048*110*15];


//void init_plots();


//pthread_t plot_thread;
//sem_t plot_sem;

uint32_t plot_sf_idx=0;
bool plot_track = true;
float rb_up[1024], rb_dw[1024], bw_up[1024], bw_dw[1024];
float* current_colored_rb_map_dw = NULL;
float* current_colored_rb_map_up = NULL;

#ifndef DISABLE_RF
#include "srslte/phy/rf/rf.h"
#include "srslte/phy/rf/rf_utils.h"

cell_search_cfg_t cell_detect_config_cni = {
  SRSLTE_DEFAULT_MAX_FRAMES_PBCH,
  SRSLTE_DEFAULT_MAX_FRAMES_PSS,
  SRSLTE_DEFAULT_NOF_VALID_PSS_FRAMES,
  0
};

#else
#warning Compiling pdsch_ue with no RF support
#endif

static bool isEqual(double a, double b, double epsilon) {
    double diff = a - b;
    return (diff < epsilon) && (diff > -epsilon);
}

//#define STDOUT_COMPACT

/**********************************************************************
 *  Program arguments processing
 ***********************************************************************/
/*
typedef struct {
  int nof_subframes;
  int cpu_affinity;
  bool disable_plots;
  bool disable_cfo;
  uint32_t time_offset;
  int force_N_id_2;
  char *input_file_name;
  char *rnti_list_file;
  char *rnti_list_file_out;
  int file_offset_time;
  float file_offset_freq;
  uint32_t file_nof_prb;
  uint32_t file_nof_ports;
  uint32_t file_cell_id;
  char *rf_args;
  uint32_t rf_nof_rx_ant;
  double rf_freq;
  float rf_gain;
  int net_port;
  char *net_address;
  int net_port_signal;
  char *net_address_signal;
  int decimate;
}prog_args_t;
*/
void args_default_cni(prog_args_t *args) {
  args->disable_plots = false;
  args->nof_subframes = -1;
  args->force_N_id_2 = -1; // Pick the best
  args->input_file_name = prog_args.input_file_name; //NULL;
  args->rnti_list_file = NULL;
  args->rnti_list_file_out = NULL;
  args->disable_cfo = false;
  args->time_offset = 0;
  args->file_nof_prb = prog_args.file_nof_prb;      //25;
  args->file_nof_ports = prog_args.file_nof_ports;  //1;
  args->file_cell_id = prog_args.file_cell_id;      //0;
  args->file_offset_time = 0;
  args->file_offset_freq = 0;
  args->rf_args = "";
  args->rf_freq = prog_args.rf_freq;  // best for test: 954900000;
  args->rf_nof_rx_ant = 1;
#ifdef ENABLE_AGC_DEFAULT
  args->rf_gain = -1.0;
#else
  args->rf_gain = 50.0;
#endif
  args->net_port = -1;
  args->net_address = "127.0.0.1";
  args->net_port_signal = -1;
  args->net_address_signal = "127.0.0.1";
  args->decimate = 0;
  args->cpu_affinity = -1;
}

void usage_cni(prog_args_t *args, char *prog) {
  printf("Usage: %s [aAgpPoOcildDnuvyYzZ] -f rx_frequency (in Hz) | -i input_file\n", prog);
#ifndef DISABLE_RF
  printf("\t-a RF args [Default %s]\n", args->rf_args);
  printf("\t-A Number of RX antennas [Default %d]\n", args->rf_nof_rx_ant);
#ifdef ENABLE_AGC_DEFAULT
  printf("\t-g RF fix RX gain [Default AGC]\n");
#else
  printf("\t-g Set RX gain [Default %.1f dB]\n", args->rf_gain);
#endif  
#else
  printf("\t   RF is disabled.\n");
#endif
  printf("\t-i input_file [Default use RF board]\n");
  printf("\t-o offset frequency correction (in Hz) for input file [Default %.1f Hz]\n", args->file_offset_freq);
  printf("\t-O offset samples for input file [Default %d]\n", args->file_offset_time);
  printf("\t-p nof_prb for input file [Default %d]\n", args->file_nof_prb);
  printf("\t-P nof_ports for input file [Default %d]\n", args->file_nof_ports);
  printf("\t-c cell_id for input file [Default %d]\n", args->file_cell_id);
  printf("\t-l Force N_id_2 [Default best]\n");
  printf("\t-C Disable CFO correction [Default %s]\n", args->disable_cfo?"Disabled":"Enabled");
  printf("\t-t Add time offset [Default %d]\n", args->time_offset);
#ifndef DISABLE_GRAPHICS
  printf("\t-d disable plots [Default enabled]\n");
#else
  printf("\t plots are disabled. Graphics library not available\n");
#endif
  printf("\t-y set the cpu affinity mask [Default %d] \n  ",args->cpu_affinity);
  printf("\t-Y set the decimate value [Default %d] \n  ",args->decimate);
  printf("\t-n nof_subframes [Default %d]\n", args->nof_subframes);
  printf("\t-s remote UDP port to send input signal (-1 does nothing with it) [Default %d]\n", args->net_port_signal);
  printf("\t-S remote UDP address to send input signal [Default %s]\n", args->net_address_signal);
  printf("\t-u remote TCP port to send data (-1 does nothing with it) [Default %d]\n", args->net_port);
  printf("\t-U remote TCP address to send data [Default %s]\n", args->net_address);
  printf("\t-v [set srslte_verbose to debug, default none]\n");
  printf("\t-z filename of the output reporting one int per rnti (tot length 64k entries)\n");
  printf("\t-Z filename of the input reporting one int per rnti (tot length 64k entries)\n");
}

bool parse_args_cni(prog_args_t *args, int argc, char **argv) {
  int opt;
  args_default_cni(args);
  while ((opt = getopt(argc, argv, "aAoglipPcOCtdnvfuUsSyYzZ")) != -1) {
    switch (opt) {
      case 'i':
        args->input_file_name = argv[optind];
        break;
      case 'p':
        args->file_nof_prb = atoi(argv[optind]);
        break;
      case 'P':
        args->file_nof_ports = atoi(argv[optind]);
        break;
      case 'o':
        args->file_offset_freq = atof(argv[optind]);
        break;
      case 'O':
        args->file_offset_time = atoi(argv[optind]);
        break;
      case 'c':
        args->file_cell_id = atoi(argv[optind]);
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
      case 't':
        args->time_offset = atoi(argv[optind]);
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
      case 'u':
        args->net_port = atoi(argv[optind]);
        break;
      case 'U':
        args->net_address = argv[optind];
        break;
      case 's':
        args->net_port_signal = atoi(argv[optind]);
        break;
      case 'S':
        args->net_address_signal = argv[optind];
        break;
      case 'd':
        args->disable_plots = true;
        break;
      case 'v':
        srslte_verbose++;
        break;
      case 'Y':
        args->decimate = atoi(argv[optind]);
        break;
      case 'y':
        args->cpu_affinity = atoi(argv[optind]);
        break;
      case 'z':
        args->rnti_list_file_out = argv[optind];
        break;
      case 'Z':
        args->rnti_list_file = argv[optind];
        break;
      default:
        usage_cni(args, argv[0]);
        return false;
    }
  }
  if (args->rf_freq < 0 && args->input_file_name == NULL) {
    usage_cni(args, argv[0]);
    return false;
  }
  return true;
}
/**********************************************************************/

/* TODO: Do something with the output data */
uint8_t *data[SRSLTE_MAX_CODEWORDS];

//bool go_exit = false;
void sig_int_handler_cni(int signo)
{
  printf("SIGINT received. Exiting...\n");
  if (signo == SIGINT) {
    go_exit = true;
  }
}

cf_t *sf_buffer[SRSLTE_MAX_PORTS] = {NULL};

#ifndef DISABLE_RF
int falcon_rf_recv_wrapper_cni(void *h, cf_t *data[SRSLTE_MAX_PORTS], uint32_t nsamples, srslte_timestamp_t *t) {
  DEBUG(" ----  Receive %d samples  ---- \n", nsamples);
  void *ptr[SRSLTE_MAX_PORTS];
  for (int i=0;i<SRSLTE_MAX_PORTS;i++) {
    ptr[i] = data[i];
  }
  return srslte_rf_recv_with_time_multi(h, ptr, nsamples, true, NULL, NULL);
}

double falcon_rf_set_rx_gain_th_wrapper_cni(void *h, double f) {
  return srslte_rf_set_rx_gain_th((srslte_rf_t*) h, f);
}

#endif

extern float mean_exec_time;

enum receiver_state { DECODE_MIB, DECODE_PDSCH} state; 

srslte_ue_dl_t ue_dl;
falcon_ue_dl_t falcon_ue_dl;
srslte_ue_sync_t ue_sync; 
//prog_args_t prog_args; 

uint32_t sfn = 0; // system frame number
srslte_netsink_t net_sink, net_sink_signal; 

#define PRINT_LINE_INIT() int this_nof_lines = 0; static int prev_nof_lines = 0
#define PRINT_LINE(_fmt, ...) printf("\033[K" _fmt "\n", ##__VA_ARGS__); this_nof_lines++
#define PRINT_LINE_RESET_CURSOR() printf("\033[%dA", this_nof_lines); prev_nof_lines = this_nof_lines
#define PRINT_LINE_ADVANCE_CURSOR() printf("\033[%dB", prev_nof_lines + 1)

int start_cni_decoder() {
  state = DECODE_MIB;
  int ret;
  int decimate = 1;
  srslte_cell_t cell;
  int64_t sf_cnt;
  srslte_ue_mib_t ue_mib;

#ifndef DISABLE_RF
  srslte_rf_t rf;
#endif
  int n;
  uint32_t last_good = 0;
  uint32_t nof_trials = 0;
  uint8_t bch_payload[SRSLTE_BCH_PAYLOAD_LEN];
  uint32_t sfn_offset;
  float cfo = 0;
  FILE *fid;
  uint16_t rnti_tmp;

  //srslte_debug_handle_crash(argc, argv);

  //parse_args(&prog_args, argc, argv);
  args_default_cni(&prog_args);

  for (int i = 0; i< SRSLTE_MAX_CODEWORDS; i++) {
    data[i] = srslte_vec_malloc(sizeof(uint8_t)*1500*8);
    if (!data[i]) {
      ERROR("Allocating data");
      go_exit = true;
    }
  }

  if(prog_args.cpu_affinity > -1) {

    /* // cpu_set_t cpuset;
   // pthread_t thread;

    thread = pthread_self();
    for(int i = 0; i < 8;i++){
      if(((prog_args.cpu_affinity >> i) & 0x01) == 1){
        printf("Setting cni_cc_decoder with affinity to core %d\n", i);
        CPU_SET((size_t) i , &cpuset);
      }
      if(pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset)){
        fprintf(stderr, "Error setting main thread affinity to %d \n", prog_args.cpu_affinity);
        return false;
      }
    } */
  }

  if (prog_args.net_port > 0) {
    if (srslte_netsink_init(&net_sink, prog_args.net_address, prog_args.net_port, SRSLTE_NETSINK_TCP)) {
      fprintf(stderr, "Error initiating UDP socket to %s:%d\n", prog_args.net_address, prog_args.net_port);
      return false;
    }
    srslte_netsink_set_nonblocking(&net_sink);
  }
  if (prog_args.net_port_signal > 0) {
    if (srslte_netsink_init(&net_sink_signal, prog_args.net_address_signal,
                            prog_args.net_port_signal, SRSLTE_NETSINK_UDP)) {
      fprintf(stderr, "Error initiating UDP socket to %s:%d\n", prog_args.net_address_signal, prog_args.net_port_signal);
      return false;
    }
    srslte_netsink_set_nonblocking(&net_sink_signal);
  }

#ifndef DISABLE_RF
  if (!prog_args.input_file_name) {

    printf("Opening RF device with %d RX antennas...\n", prog_args.rf_nof_rx_ant);
    if (srslte_rf_open_multi(&rf, prog_args.rf_args, prog_args.rf_nof_rx_ant)) {
      fprintf(stderr, "Error opening rf\n");
      return false;
    }
    /* Set receiver gain */
    if (prog_args.rf_gain > 0) {
      srslte_rf_set_rx_gain(&rf, prog_args.rf_gain);
    } else {
      printf("Starting AGC thread...\n");
      if (srslte_rf_start_gain_thread(&rf, false)) {
        fprintf(stderr, "Error opening rf\n");
        return false;
      }
      srslte_rf_set_rx_gain(&rf, srslte_rf_get_rx_gain(&rf));
      cell_detect_config_cni.init_agc = srslte_rf_get_rx_gain(&rf);
    }

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    signal(SIGINT, sig_int_handler_cni);

    srslte_rf_set_master_clock_rate(&rf, 30.72e6);

    /* set receiver frequency */
    printf("Tunning receiver to %.3f MHz\n", prog_args.rf_freq/1000000);
    srslte_rf_set_rx_freq(&rf, prog_args.rf_freq);
    srslte_rf_rx_wait_lo_locked(&rf);

    uint32_t ntrial=0;
    do {
      ret = rf_search_and_decode_mib(&rf, prog_args.rf_nof_rx_ant, &cell_detect_config_cni, prog_args.force_N_id_2, &cell, &cfo);
      if (ret < 0) {
        fprintf(stderr, "Error searching for cell\n");
        return false;
      } else if (ret == 0 && !go_exit) {
        printf("Cell not found after %d trials. Trying again (Press Ctrl+C to exit)\n", ntrial++);
      }
    } while (ret == 0 && !go_exit);

    if (go_exit) {
      srslte_rf_close(&rf);
      //exit(0);
      return true;
    }

    srslte_rf_stop_rx_stream(&rf);
    //srslte_rf_flush_buffer(&rf);

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
        return false;
      }
    } else {
      fprintf(stderr, "Invalid number of PRB %d\n", cell.nof_prb);
      return false;
    }

    INFO("Stopping RF and flushing buffer...\r");
  }
#endif

  /* If reading from file, go straight to PDSCH decoding. Otherwise, decode MIB first */
  if (prog_args.input_file_name) {
    /* preset cell configuration */
    cell.id = prog_args.file_cell_id;
    cell.cp = SRSLTE_CP_NORM;
    cell.phich_length = SRSLTE_PHICH_NORM;
    cell.phich_resources = SRSLTE_PHICH_R_1;
    cell.nof_ports = prog_args.file_nof_ports;
    cell.nof_prb = prog_args.file_nof_prb;

    if (srslte_ue_sync_init_file(&ue_sync,
                                 prog_args.file_nof_prb,
                                 prog_args.input_file_name,
                                 prog_args.file_offset_time,
                                 prog_args.file_offset_freq)) {
      fprintf(stderr, "Error initiating ue_sync\n");
      return false;
    }

  }
  else {
#ifndef DISABLE_RF
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
                                        falcon_rf_recv_wrapper_cni,
                                        prog_args.rf_nof_rx_ant,
                                        (void*) &rf, decimate)) {
      fprintf(stderr, "Error initiating ue_sync\n");
      return false;
    }
    if (srslte_ue_sync_set_cell(&ue_sync, cell)) {
      fprintf(stderr, "Error initiating ue_sync\n");
      return false;
    }
#endif
  }

  for (int i=0;i<prog_args.rf_nof_rx_ant;i++) {
    sf_buffer[i] = srslte_vec_malloc(3*sizeof(cf_t)*SRSLTE_SF_LEN_PRB(cell.nof_prb));
  }

  if (srslte_ue_mib_init(&ue_mib, sf_buffer, cell.nof_prb)) {
    fprintf(stderr, "Error initaiting UE MIB decoder\n");
    return false;
  }
  if (srslte_ue_mib_set_cell(&ue_mib, cell)) {
    fprintf(stderr, "Error initaiting UE MIB decoder\n");
    return false;
  }

  if (falcon_ue_dl_init(&falcon_ue_dl,
                        &ue_dl,
                        sf_buffer,
                        cell.nof_prb,
                        prog_args.rf_nof_rx_ant,
                        "",
                        "",
                        false))
  {
    //if (srslte_ue_dl_init(&ue_dl, sf_buffer, cell.nof_prb, prog_args.rf_nof_rx_ant)) {
    fprintf(stderr, "Error initiating UE downlink processing module\n");
    return false;
  }
  //########################## MO HACKS ####################

  falcon_ue_dl.decoderthread = decoderthread;

  //########################## MO HACKS ####################

  if (srslte_ue_dl_set_cell(&ue_dl, cell)) {
    fprintf(stderr, "Error initiating UE downlink processing module\n");
    return false;
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

  if (!prog_args.disable_plots) {
    //    init_plots(cell);                                         //MO_HACKS
    bzero(rb_up, 1024*sizeof(float));
    bzero(rb_dw, 1024*sizeof(float));
    bzero(bw_up, 1024*sizeof(float));
    bzero(bw_dw, 1024*sizeof(float));
  }

#ifndef DISABLE_RF
  if (!prog_args.input_file_name) {
    srslte_rf_start_rx_stream(&rf, false);
  }
#endif

#ifndef DISABLE_RF
  if (prog_args.rf_gain < 0 && !prog_args.input_file_name) {
    srslte_rf_info_t *rf_info = srslte_rf_get_info(&rf);
    srslte_ue_sync_start_agc(&ue_sync,
                             falcon_rf_set_rx_gain_th_wrapper_cni,
                             rf_info->min_rx_gain,
                             rf_info->max_rx_gain,
                             cell_detect_config_cni.init_agc);
  }
#endif

  ue_sync.cfo_correct_enable_track = !prog_args.disable_cfo;

  falcon_ue_dl.q = &ue_dl;
  if (prog_args.rnti_list_file != NULL) { // loading RNTI list
    fid = fopen(prog_args.rnti_list_file,"r");
    if (fid>0) {
      for (int i=0;i<65536;i++) {
        if (fscanf(fid,"%hu",&rnti_tmp) != EOF) {
          //		  if (rnti_tmp) printf("rnti %d val %d\n", i, rnti_tmp);
          srslte_ue_dl_reset_rnti_user_to(&falcon_ue_dl, i, rnti_tmp); // check this
          //printf("is rnti in the list? %d\n",rnti_in_list(&ue_dl, rnti_tmp));
        }
      }
      fclose(fid);
    }
  }

  srslte_pbch_decode_reset(&ue_mib.pbch);

  // moved to falcon_ue_dl_init()
  //  // Initialize Histogram
  //  for(int hst=0; hst<NOF_UE_ALL_FORMATS; hst++)
  //      rnti_histogram_init(&falcon_ue_dl.rnti_histogram[hst]);

  INFO("\nEntering main loop...\n\n", 0);
  /* Main loop */
  while (!go_exit && (sf_cnt < prog_args.nof_subframes || prog_args.nof_subframes == -1)) {

//    if(sf_cnt % (DEFAULT_DCI_FORMAT_SPLIT_UPDATE_INTERVAL_MS) == 0) {
//      falcon_ue_dl_update_formats(&falcon_ue_dl, DEFAULT_DCI_FORMAT_SPLIT_RATIO);
//    }

    bool acks [SRSLTE_MAX_CODEWORDS] = {false};
    ret = srslte_ue_sync_zerocopy_multi(&ue_sync, sf_buffer);
    if (ret < 0) {
      fprintf(stderr, "Error calling srslte_ue_sync_work()\n");
    }
    /* TODO: Add return value 7 for controlled shutdown after reading whole file in file mode */
    if (ret == 7) {
      if (!prog_args.disable_plots) {
        plot_sf_idx = sfn % 1024;
        last_good = sfn;
        plot_track = true;
        //sem_post(&plot_sem);
      }
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
            n = srslte_ue_mib_decode(&ue_mib, bch_payload, NULL, &sfn_offset);
            if (n < 0) {
              fprintf(stderr, "Error decoding UE MIB\n");
              return false;
            } else if (n == SRSLTE_UE_MIB_FOUND) {
              srslte_pbch_mib_unpack(bch_payload, &cell, &sfn);
              srslte_cell_fprint(stdout, &cell, sfn);
              printf("Decoded MIB. SFN: %d, offset: %d\n", sfn, sfn_offset);
              sfn = (sfn + sfn_offset)%1024;
              state = DECODE_PDSCH;
              if(falcon_ue_dl.rnti_manager != 0) {
                rnti_manager_free(falcon_ue_dl.rnti_manager);
                falcon_ue_dl.rnti_manager = 0;
              }
              falcon_ue_dl.rnti_manager = rnti_manager_create(nof_falcon_ue_all_formats, RNTI_PER_SUBFRAME);
              // setup rnti manager
              int idx;
              // add format1A evergreens
              idx = falcon_dci_index_of_format_in_list(SRSLTE_DCI_FORMAT1A, falcon_ue_all_formats, nof_falcon_ue_all_formats);
              if(idx > -1) {
                rnti_manager_add_evergreen(falcon_ue_dl.rnti_manager, SRSLTE_RARNTI_START, SRSLTE_RARNTI_END, (uint32_t)idx);
                rnti_manager_add_evergreen(falcon_ue_dl.rnti_manager, SRSLTE_PRNTI, SRSLTE_SIRNTI, (uint32_t)idx);
              }
              // add format1C evergreens
              idx = falcon_dci_index_of_format_in_list(SRSLTE_DCI_FORMAT1C, falcon_ue_all_formats, nof_falcon_ue_all_formats);
              if(idx > -1) {
                rnti_manager_add_evergreen(falcon_ue_dl.rnti_manager, SRSLTE_RARNTI_START, SRSLTE_RARNTI_END, (uint32_t)idx);
                rnti_manager_add_evergreen(falcon_ue_dl.rnti_manager, SRSLTE_PRNTI, SRSLTE_SIRNTI, (uint32_t)idx);
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
          srslte_ue_dl_get_control_cc_hist(&falcon_ue_dl, srslte_ue_sync_get_sfidx(&ue_sync), sfn);
          if (ue_dl.current_rnti != 0xffff) {
            //n = srslte_ue_dl_decode_broad(&ue_dl, &sf_buffer[prog_args.time_offset], data, srslte_ue_sync_get_sfidx(&ue_sync), ue_dl.current_rnti);
            if(cell.nof_ports == 1) {
              /* Transmission mode 1 */
              n = srslte_ue_dl_decode(&ue_dl, data, 0, sfn * 10 + srslte_ue_sync_get_sfidx(&ue_sync), acks);
            }
            else {
              /* Transmission mode 2 */
              n = srslte_ue_dl_decode(&ue_dl, data, 1, sfn * 10 + srslte_ue_sync_get_sfidx(&ue_sync), acks);
            }

            switch(n) {
              case 0:
                //        			printf("No decode\n");
                break;
              case 40:
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)data[(n/8)-2] + data[(n/8)-1]);
                //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*data[(n/8)-2] + data[(n/8)-1]);
                //        			for (int k=0; k<(n/8); k++) {
                //        				printf("%02x ",data[k]);
                //        			}
                //        			printf("\n");
                break;
              case 56:
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)data[(n/8)-2] + data[(n/8)-1]);
                //        			printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*data[(n/8)-2] + data[(n/8)-1]);
                //        			for (int k=0; k<(n/8); k++) {
                //        				printf("%02x ",data[k]);
                //        			}
                //        			printf("\n");
                break;
              case 72:
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)data[(n/8)-4] + data[(n/8)-3]);
                //        			printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*data[(n/8)-4] + data[(n/8)-3]);
                //        			for (int k=0; k<(n/8); k++) {
                //        				printf("%02x ",data[k]);
                //        			}
                //        			printf("\n");
                break;
              case 120:
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)data[(n/8)-3] + data[(n/8)-2]);
                srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)data[(n/8)-9] + data[(n/8)-8]);
                //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*data[(n/8)-9] + data[(n/8)-8]);
                //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*data[(n/8)-3] + data[(n/8)-2]);
                //					for (int k=0; k<(n/8); k++) {
                //						printf("%02x ",data[k]);
                //					}
                //					printf("\n");
                break;
              default:
                //        			fprintf(stderr,"\n");
                //					for (int k=0; k<(n/8); k++) {
                //						fprintf(stderr,"%02x ",data[k]);
                //					}
                //					fprintf(stderr,"\n");
                break;
            }
          }
          break;
      }
      if (!prog_args.disable_plots) {

        //      printf("%d %d %d %d\n", ue_dl.totRBup, ue_dl.totRBdw, ue_dl.totBWup, ue_dl.totBWdw);
        if (plot_track == false) {
          for (int i = last_good+1; i<sfn; i++) {
            rb_up[i % 1024] += 0;
            rb_dw[i % 1024] += 0;
            bw_up[i % 1024] += 0;
            bw_dw[i % 1024] += 0;
          }
        }

        rb_up[sfn % 1024] += ((float) falcon_ue_dl.totRBup)/10;
        rb_dw[sfn % 1024] += ((float) falcon_ue_dl.totRBdw)/10;
        bw_up[sfn % 1024] += ((float) falcon_ue_dl.totBWup)/10;
        bw_dw[sfn % 1024] += ((float) falcon_ue_dl.totBWdw)/10;
      }
      if (srslte_ue_sync_get_sfidx(&ue_sync) == 9) {
        sfn++;
        if (!prog_args.disable_plots) {
          rb_up[sfn % 1024] = 0;
          rb_dw[sfn % 1024] = 0;
          bw_up[sfn % 1024] = 0;
          bw_dw[sfn % 1024] = 0;
        }
        if (sfn % 1024 == 0) {
          srslte_ue_dl_update_rnti_list(&falcon_ue_dl);
        }
      }
      if (!prog_args.disable_plots) {
        if ((sfn%PLOT_REFRESH_SFN) == 0 && state == DECODE_PDSCH) {
          plot_sf_idx = sfn % 1024;
          last_good = sfn;
          plot_track = true;
          current_colored_rb_map_dw = falcon_ue_dl.colored_rb_map_dw_last;
          current_colored_rb_map_up = falcon_ue_dl.colored_rb_map_up_last;


          bzero(tmp_plot_wf,12*ue_dl.cell.nof_prb*sizeof(float));
          for (int j = 0; j < 14; j++) {
            for (int i = 0; i < 12*ue_dl.cell.nof_prb; i++) {
              tmp_plot_wf[i] += 20 * log10f(cabsf(ue_dl.sf_symbols[i+j*(12*ue_dl.cell.nof_prb)]))/14;
            }
          }

          float tmp_linebuffer[200]; //200 should be sufficient
          float tmp_buff = 0;

          for(int i = 0; i < ue_dl.cell.nof_prb; i++){

            for(int ii = 0; ii < 12; ii++){

              tmp_buff += tmp_plot_wf[(i*12)+ii];
            }

            tmp_linebuffer[i] = tmp_buff * 128;

            tmp_buff = 0;

          }

          uint32_t hist_int[65536];
          //uint64_t sum = 0;
          rnti_manager_get_histogram_summary(falcon_ue_dl.rnti_manager, hist_int);
          /*  for(j = 0; j < 65536; j++) {
            tmp_hist[j] = hist_int[j];
            //sum += hist_int[j];
          }*/




          plot_scanLines(decoderthread,falcon_ue_dl.colored_rb_map_up_last, falcon_ue_dl.colored_rb_map_dw_last,tmp_linebuffer,hist_int);

          //sem_post(&plot_sem);
        }
      }
#ifdef __LOG_ACTIVE_SET__
      if (sfn % 10 == 0 && srslte_ue_sync_get_sfidx(&ue_sync) == 9) {
        rnti_manager_active_set_t active[100];
        uint32_t n_active = 100;
        n_active = rnti_manager_get_active_set(falcon_ue_dl.rnti_manager, active, n_active);
        for(uint32_t i = 0; i< n_active; i++) {
          printf("%d: %d idx: %d\n", active[i].rnti, active[i].frequency, active[i].assoc_format_idx);
        }
      }
#endif
    } else if (ret == 0) {
      printf("Finding PSS... Peak: %8.1f, FrameCnt: %d, State: %d\r",
             srslte_sync_get_peak_value(&ue_sync.sfind),
             ue_sync.frame_total_cnt, ue_sync.state);
      if (!prog_args.disable_plots) {
        //      	plot_sf_idx = srslte_ue_sync_get_sfidx(&ue_sync);
        plot_track = false;
        //      	sem_post(&plot_sem);
      }
    }
    // Some delay when playing
    if (prog_args.input_file_name) {
      //usleep(10000);
    }
    sf_cnt++;
  } // Main loop

  srslte_ue_dl_stats_print(&falcon_ue_dl, falcon_ue_dl.stats_file);
  //sleep(5);
  if (!prog_args.disable_plots) {
    /*if (!pthread_kill(plot_thread, 0)) {
      //pthread_kill(plot_thread, SIGHUP);
      pthread_cancel(plot_thread);
      pthread_join(plot_thread, NULL);
    }*/
  }

  if (prog_args.rnti_list_file_out != NULL) {
    fid = fopen(prog_args.rnti_list_file_out,"w");
    for (int i=0;i<65536;i++) {
      if (i<=10) {
        fprintf(fid,"%d\n",2);
      } else {
        //			if (rnti_in_list(&ue_dl, i)) printf("%d val %d (%d)\n",i,ue_dl.rnti_list[i], ue_dl.rnti_cnt[i]);
        if (falcon_ue_dl.rnti_cnt[i] >= 10) fprintf(fid,"%d\n",falcon_ue_dl.rnti_list[i]);
        else fprintf(fid,"%d\n",0);
      }
    }
    fclose(fid);
  }

  rnti_manager_free(falcon_ue_dl.rnti_manager);
  falcon_ue_dl_free(&falcon_ue_dl);
  srslte_ue_sync_free(&ue_sync);

#ifndef DISABLE_RF
  if (!prog_args.input_file_name) {
    srslte_ue_mib_free(&ue_mib);
    srslte_rf_close(&rf);
  }
#endif
  //exit(0);
  return true;
}

