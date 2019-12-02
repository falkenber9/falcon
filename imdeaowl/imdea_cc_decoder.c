/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2015 Software Radio Systems Limited
 * Copyright 2016 IMDEA Networks Institute
 * Copyright 2018-2019 TU Dortmund University, Communication Networks Institute
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

#include "srslte/common/crash_handler.h"
#include "srslte/srslte.h"
#include "falcon/phy/falcon_ue/falcon_ue_dl.h"

#define PLOT_REFRESH_SFN 1
#define ENABLE_AGC_DEFAULT
//#define CORRECT_SAMPLE_OFFSET

#define HISTOGRAM_NOT_USED_BY_THIS_PROGRAM 99999

#ifdef ENABLE_GUI
#include "srsgui/srsgui.h"
#endif
void init_plots(void);
static pthread_t plot_thread;
static sem_t plot_sem;
static uint32_t plot_sf_idx=0;
static bool plot_track = true;
static float rb_up[1024], rb_dw[1024], bw_up[1024], bw_dw[1024];
static float* current_colored_rb_map_dw = NULL;
static float* current_colored_rb_map_up = NULL;

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

/**********************************************************************
 *  Program arguments processing
 ***********************************************************************/
typedef struct {
  int nof_subframes;
  int cpu_affinity;
  bool disable_plots;
  bool disable_cfo;
  bool reencoding_only;
  uint32_t time_offset;
  int force_N_id_2;
  char *input_file_name;
  char *dci_file_name;
  char *stats_file_name;
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
} prog_args_t;

void args_default(prog_args_t *args) {
  args->disable_plots = false;
  args->nof_subframes = -1;
  args->force_N_id_2 = -1; // Pick the best
  args->input_file_name = NULL;
  args->dci_file_name = "";
  args->stats_file_name = "";
  args->rnti_list_file = NULL;
  args->rnti_list_file_out = NULL;
  args->disable_cfo = false;
  args->reencoding_only = false;
  args->time_offset = 0;
  args->file_nof_prb = 25;
  args->file_nof_ports = 1;
  args->file_cell_id = 0;
  args->file_offset_time = 0;
  args->file_offset_freq = 0;
  args->rf_args = "";
  args->rf_freq = -1.0;
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

void usage(prog_args_t *args, char *prog) {
  printf("Usage: %s [aAgrpPoOcildDnuvyYzZ] -f rx_frequency (in Hz) | -i input_file\n", prog);
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
  printf("\t-D output filename for DCI [default stdout]\n");
  printf("\t-E output filename for statistics [default stdout]\n");
  printf("\t-r reencoding only (LTEye mode)\n");
  printf("\t-o offset frequency correction (in Hz) for input file [Default %.1f Hz]\n", (double)args->file_offset_freq);
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

void parse_args(prog_args_t *args, int argc, char **argv) {
  int opt;
  args_default(args);
  while ((opt = getopt(argc, argv, "aADrEoglipPcOCtdnvfuUsSyYzZ")) != -1) {
    switch (opt) {
      case 'i':
        args->input_file_name = argv[optind];
        break;
      case 'D':
        args->dci_file_name = argv[optind];
        break;
      case 'E':
        args->stats_file_name = argv[optind];
        break;
      case 'r':
        args->reencoding_only = true;
        break;
      case 'p':
        args->file_nof_prb = (uint32_t)atoi(argv[optind]);
        break;
      case 'P':
        args->file_nof_ports = (uint32_t)atoi(argv[optind]);
        break;
      case 'o':
        args->file_offset_freq = (float)atof(argv[optind]);
        break;
      case 'O':
        args->file_offset_time = atoi(argv[optind]);
        break;
      case 'c':
        args->file_cell_id = (uint32_t)atoi(argv[optind]);
        break;
      case 'a':
        args->rf_args = argv[optind];
        break;
      case 'A':
        args->rf_nof_rx_ant = (uint32_t)atoi(argv[optind]);
        break;
      case 'g':
        args->rf_gain = (float)atof(argv[optind]);
        break;
      case 'C':
        args->disable_cfo = true;
        break;
      case 't':
        args->time_offset = (uint32_t)atoi(argv[optind]);
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
        usage(args, argv[0]);
        exit(-1);
    }
  }
  if (args->rf_freq < 0 && args->input_file_name == NULL) {
    usage(args, argv[0]);
    exit(-1);
  }
}
/**********************************************************************/

/* Buffers for PCH reception (not included in DL HARQ) */
const static uint32_t  pch_payload_buffer_sz = 8*1024;  // cf. srslte: srsue/hdr/mac/mac.h
static uint8_t *pch_payload_buffers[SRSLTE_MAX_CODEWORDS];

static bool go_exit = false;

void sig_int_handler(int signo)
{
  printf("SIGINT received. Exiting...\n");
  if (signo == SIGINT) {
    go_exit = true;
  }
}

static cf_t *sf_buffer[SRSLTE_MAX_PORTS] = {NULL};

#ifndef DISABLE_RF
int srslte_rf_recv_wrapper(void *h, cf_t *data[SRSLTE_MAX_PORTS], uint32_t nsamples, srslte_timestamp_t *t) {
  (void)t;
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

#endif

extern float mean_exec_time;

static enum receiver_state { DECODE_MIB, DECODE_PDSCH} state;

static srslte_ue_dl_t ue_dl;
static falcon_ue_dl_t falcon_ue_dl;
static srslte_ue_sync_t ue_sync;
static prog_args_t prog_args;

static uint32_t sfn = 0; // system frame number
static srslte_netsink_t net_sink, net_sink_signal;

#define PRINT_LINE_INIT() int this_nof_lines = 0; static int prev_nof_lines = 0
#define PRINT_LINE(_fmt, ...) printf("\033[K" _fmt "\n", ##__VA_ARGS__); this_nof_lines++
#define PRINT_LINE_RESET_CURSOR() printf("\033[%dA", this_nof_lines); prev_nof_lines = this_nof_lines
#define PRINT_LINE_ADVANCE_CURSOR() printf("\033[%dB", prev_nof_lines + 1)

int main(int argc, char **argv) {
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
  uint8_t bch_payload[SRSLTE_BCH_PAYLOAD_LEN];
  int32_t sfn_offset;
  float cfo = 0;
  FILE *fid;
  uint16_t rnti_tmp;

  srslte_debug_handle_crash(argc, argv);

  parse_args(&prog_args, argc, argv);

  for (int i = 0; i< SRSLTE_MAX_CODEWORDS; i++) {
    pch_payload_buffers[i] = srslte_vec_malloc(sizeof(uint8_t)*pch_payload_buffer_sz);
    if (!pch_payload_buffers[i]) {
      ERROR("Allocating pch_payload_buffers");
      go_exit = true;
    }
  }

  if(prog_args.cpu_affinity > -1) {

    cpu_set_t cpuset;
    pthread_t thread;

    thread = pthread_self();
    for(int i = 0; i < 8;i++){
      if(((prog_args.cpu_affinity >> i) & 0x01) == 1){
        printf("Setting imdea_cc_decoder with affinity to core %d\n", i);
        CPU_SET((size_t) i , &cpuset);
      }
      if(pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset)){
        fprintf(stderr, "Error setting main thread affinity to %d \n", prog_args.cpu_affinity);
        exit(-1);
      }
    }
  }

  if (prog_args.net_port > 0) {
    if (srslte_netsink_init(&net_sink, prog_args.net_address, (uint16_t)prog_args.net_port, SRSLTE_NETSINK_TCP)) {
      fprintf(stderr, "Error initiating UDP socket to %s:%d\n", prog_args.net_address, prog_args.net_port);
      exit(-1);
    }
    srslte_netsink_set_nonblocking(&net_sink);
  }
  if (prog_args.net_port_signal > 0) {
    if (srslte_netsink_init(&net_sink_signal, prog_args.net_address_signal,
                            (uint16_t)prog_args.net_port_signal, SRSLTE_NETSINK_UDP)) {
      fprintf(stderr, "Error initiating UDP socket to %s:%d\n", prog_args.net_address_signal, prog_args.net_port_signal);
      exit(-1);
    }
    srslte_netsink_set_nonblocking(&net_sink_signal);
  }

#ifndef DISABLE_RF
  if (!prog_args.input_file_name) {

    printf("Opening RF device with %d RX antennas...\n", prog_args.rf_nof_rx_ant);
    if (srslte_rf_open_multi(&rf, prog_args.rf_args, prog_args.rf_nof_rx_ant)) {
      fprintf(stderr, "Error opening rf\n");
      exit(-1);
    }
    /* Set receiver gain */
    if (prog_args.rf_gain > 0) {
      srslte_rf_set_rx_gain(&rf, (double)prog_args.rf_gain);
    } else {
      printf("Starting AGC thread...\n");
      if (srslte_rf_start_gain_thread(&rf, false)) {
        fprintf(stderr, "Error opening rf\n");
        exit(-1);
      }
      srslte_rf_set_rx_gain(&rf, srslte_rf_get_rx_gain(&rf));
      cell_detect_config.init_agc = (float)srslte_rf_get_rx_gain(&rf);
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
    do {
      ret = rf_search_and_decode_mib(&rf, prog_args.rf_nof_rx_ant, &cell_detect_config, prog_args.force_N_id_2, &cell, &cfo);
      if (ret < 0) {
        fprintf(stderr, "Error searching for cell\n");
        exit(-1);
      } else if (ret == 0 && !go_exit) {
        printf("Cell not found after %d trials. Trying again (Press Ctrl+C to exit)\n", ntrial++);
      }
    } while (ret == 0 && !go_exit);

    if (go_exit) {
      srslte_rf_close(&rf);
      exit(0);
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
      printf("Setting sampling rate %.2f MHz\n", (double) srate/1000000);
      float srate_rf = (float)srslte_rf_set_rx_srate(&rf, (double) srate);
      if (!isEqual((double)srate_rf, srate, 1.0)) {
        fprintf(stderr, "Could not set sampling rate\n");
        exit(-1);
      }
    } else {
      fprintf(stderr, "Invalid number of PRB %d\n", cell.nof_prb);
      exit(-1);
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
      exit(-1);
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
                                        srslte_rf_recv_wrapper,
                                        prog_args.rf_nof_rx_ant,
                                        (void*) &rf, decimate)) {
      fprintf(stderr, "Error initiating ue_sync\n");
      exit(-1);
    }
    if (srslte_ue_sync_set_cell(&ue_sync, cell)) {
      fprintf(stderr, "Error initiating ue_sync\n");
      exit(-1);
    }
#endif
  }

  for (uint32_t i=0; i<prog_args.rf_nof_rx_ant; i++) {
    sf_buffer[i] = srslte_vec_malloc(3*sizeof(cf_t)*((uint32_t)SRSLTE_SF_LEN_PRB(cell.nof_prb)));
  }

  if (srslte_ue_mib_init(&ue_mib, sf_buffer, cell.nof_prb)) {
    fprintf(stderr, "Error initaiting UE MIB decoder\n");
    exit(-1);
  }
  if (srslte_ue_mib_set_cell(&ue_mib, cell)) {
    fprintf(stderr, "Error initaiting UE MIB decoder\n");
    exit(-1);
  }

  if (falcon_ue_dl_init(&falcon_ue_dl,
                        &ue_dl,
                        sf_buffer,
                        cell.nof_prb,
                        prog_args.rf_nof_rx_ant,
                        prog_args.dci_file_name,
                        prog_args.stats_file_name,
                        false,
                        HISTOGRAM_NOT_USED_BY_THIS_PROGRAM))
  {
  //if (srslte_ue_dl_init(&ue_dl, sf_buffer, cell.nof_prb, prog_args.rf_nof_rx_ant)) {
    fprintf(stderr, "Error initiating UE downlink processing module\n");
    exit(-1);
  }
  if (srslte_ue_dl_set_cell(&ue_dl, cell)) {
    fprintf(stderr, "Error initiating UE downlink processing module\n");
    exit(-1);
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
    init_plots();
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
                             srslte_rf_set_rx_gain_th_wrapper_,
                             rf_info->min_rx_gain,
                             rf_info->max_rx_gain,
                             (double)cell_detect_config.init_agc);
  }
#endif

  ue_sync.cfo_correct_enable_track = !prog_args.disable_cfo;

  falcon_ue_dl.q = &ue_dl;
  if (prog_args.rnti_list_file != NULL) { // loading RNTI list
    fid = fopen(prog_args.rnti_list_file,"r");
    if (fid != NULL) {
      for (int i=0;i<65536;i++) {
        if (fscanf(fid,"%hu",&rnti_tmp) != EOF) {
          //		  if (rnti_tmp) printf("rnti %d val %d\n", i, rnti_tmp);
          srslte_ue_dl_reset_rnti_user_to(&falcon_ue_dl, (uint16_t)i, rnti_tmp); // check this
          //printf("is rnti in the list? %d\n",rnti_in_list(&ue_dl, rnti_tmp));
        }
      }
      fclose(fid);
    }
  }

  srslte_pbch_decode_reset(&ue_mib.pbch);

  // Initialize Histogram
  for(int hst=0; hst<nof_falcon_ue_all_formats; hst++)
      rnti_histogram_init(&falcon_ue_dl.rnti_histogram[hst], HISTOGRAM_NOT_USED_BY_THIS_PROGRAM);

  INFO("\nEntering main loop...\n\n");
  /* Main loop */
  while (!go_exit && (sf_cnt < prog_args.nof_subframes || prog_args.nof_subframes == -1)) {

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
        sem_post(&plot_sem);
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
              exit(-1);
            } else if (n == SRSLTE_UE_MIB_FOUND) {
              srslte_pbch_mib_unpack(bch_payload, &cell, &sfn);
              srslte_cell_fprint(stdout, &cell, sfn);
              printf("Decoded MIB. SFN: %d, offset: %d\n", sfn, sfn_offset);
              sfn = (sfn + (uint32_t)sfn_offset)%1024;
              state = DECODE_PDSCH;
            }
          }
          break;
        case DECODE_PDSCH:
          srslte_ue_dl_get_control_cc(&falcon_ue_dl, srslte_ue_sync_get_sfidx(&ue_sync), sfn, prog_args.reencoding_only);
          if(!prog_args.reencoding_only) {
            if (ue_dl.current_rnti != 0xffff) {
              //n = srslte_ue_dl_decode_broad(&ue_dl, &sf_buffer[prog_args.time_offset], data, srslte_ue_sync_get_sfidx(&ue_sync), ue_dl.current_rnti);
              if(cell.nof_ports == 1) {
                /* Transmission mode 1 */
                n = srslte_ue_dl_decode(&ue_dl, pch_payload_buffers, 0, sfn * 10 + srslte_ue_sync_get_sfidx(&ue_sync), acks);
              }
              else {
                /* Transmission mode 2 */
                n = srslte_ue_dl_decode(&ue_dl, pch_payload_buffers, 1, sfn * 10 + srslte_ue_sync_get_sfidx(&ue_sync), acks);
              }
              uint8_t* pch_payload = pch_payload_buffers[0];

              switch(n) {
                case 0:
                  //        			printf("No decode\n");
                  break;
                case 40:
                  srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)pch_payload[(n/8)-2] + pch_payload[(n/8)-1]);
                  //					printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-2] + pch_payload[(n/8)-1]);
                  //        			for (int k=0; k<(n/8); k++) {
                  //        				printf("%02x ",pch_payload[k]);
                  //        			}
                  //        			printf("\n");
                  break;
                case 56:
                  srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)pch_payload[(n/8)-2] + pch_payload[(n/8)-1]);
                  //        			printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-2] + pch_payload[(n/8)-1]);
                  //        			for (int k=0; k<(n/8); k++) {
                  //        				printf("%02x ",pch_payload[k]);
                  //        			}
                  //        			printf("\n");
                  break;
                case 72:
                  srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)pch_payload[(n/8)-4] + pch_payload[(n/8)-3]);
                  //        			printf("Found %d\t%d\t%d\n", sfn, srslte_ue_sync_get_sfidx(&ue_sync), 256*pch_payload[(n/8)-4] + pch_payload[(n/8)-3]);
                  //        			for (int k=0; k<(n/8); k++) {
                  //        				printf("%02x ",pch_payload[k]);
                  //        			}
                  //        			printf("\n");
                  break;
                case 120:
                  srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)pch_payload[(n/8)-3] + pch_payload[(n/8)-2]);
                  srslte_ue_dl_reset_rnti_user(&falcon_ue_dl, 256*(uint16_t)pch_payload[(n/8)-9] + pch_payload[(n/8)-8]);
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
          } // !reencoding_only
          break;
      }
      if (!prog_args.disable_plots) {

        //      printf("%d %d %d %d\n", ue_dl.totRBup, ue_dl.totRBdw, ue_dl.totBWup, ue_dl.totBWdw);
        if (plot_track == false) {
          for (uint32_t i = last_good+1; i<sfn; i++) {
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
          sem_post(&plot_sem);
        }
      }
    } else if (ret == 0) {
      printf("Finding PSS... Peak: %8.1f, FrameCnt: %d, State: %d\r",
             (double)srslte_sync_get_peak_value(&ue_sync.sfind),
             ue_sync.frame_total_cnt, ue_sync.state);
      if (!prog_args.disable_plots) {
        //      	plot_sf_idx = srslte_ue_sync_get_sfidx(&ue_sync);
        plot_track = false;
        //      	sem_post(&plot_sem);
      }
    }
    // Some delay when playing
    if (prog_args.input_file_name && !prog_args.disable_plots) {
        usleep(1000);
    }
    sf_cnt++;
  } // Main loop

  srslte_ue_dl_stats_print(&falcon_ue_dl, falcon_ue_dl.stats_file);

  if (!prog_args.disable_plots) {
    sleep(5);
    if (!pthread_kill(plot_thread, 0)) {
      //pthread_kill(plot_thread, SIGHUP);
      pthread_cancel(plot_thread);
      pthread_join(plot_thread, NULL);
    }
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

  printf("Free Memory...\n");
  falcon_ue_dl_free(&falcon_ue_dl);
  //srslte_ue_dl_free(&ue_dl);
  srslte_ue_sync_free(&ue_sync);

#ifndef DISABLE_RF
  if (!prog_args.input_file_name) {
    srslte_ue_mib_free(&ue_mib);
    srslte_rf_close(&rf);
  }
#endif
  exit(0);
}


/**********************************************************************
 *  Plotting Functions
 ***********************************************************************/
#define GUI_ENABLE_SPECTROGRAM
//#define GUI_ENABLE_PERFORMANCEPLOTS
#define GUI_ENABLE_RBALLOC_PLOT
#define GUI_ENABLE_HISTOGRAMPLOT

#define NUM_HIST 1

static plot_waterfall_t poutfft, rb_allocs_dw, rb_allocs_up;
static plot_real_t hist[NUM_HIST];
#ifdef GUI_ENABLE_PERFORMANCEPLOTS
static plot_real_t p_rb_up, p_rb_dw, p_bw_up, p_bw_dw;
#endif

static float tmp_plot_wf[2048*110*15];
#ifdef GUI_ENABLE_PERFORMANCEPLOTS
static float tmp_plot[1024],tmp_plot2[1024],tmp_plot3[1024],tmp_plot4[1024];
#endif
static float tmp_hist[65536];

void *plot_thread_run(void *arg) {
    (void)arg;
    uint32_t i, j;
    sdrgui_init();

#ifdef GUI_ENABLE_SPECTROGRAM
    plot_waterfall_init(&poutfft, SRSLTE_NRE * (int)ue_dl.cell.nof_prb, 100);
    plot_waterfall_setTitle(&poutfft, "Output FFT - Magnitude");
    plot_waterfall_setPlotYAxisScale(&poutfft, -40, 40);
    plot_waterfall_setSpectrogramZAxisScale(&poutfft, -40, 20);
#endif

#ifdef GUI_ENABLE_RBALLOC_PLOT
    plot_waterfall_init(&rb_allocs_dw, (int)ue_dl.cell.nof_prb, 100);
    plot_waterfall_setTitle(&rb_allocs_dw, "Resource Allocations (Downlink)");
    plot_waterfall_setPlotYAxisScale(&rb_allocs_dw, 0, 65536);
    plot_waterfall_setSpectrogramZAxisScale(&rb_allocs_dw, 0, 65536);

    plot_waterfall_init(&rb_allocs_up, (int)ue_dl.cell.nof_prb, 100);
    plot_waterfall_setTitle(&rb_allocs_up, "Resource Allocations (Uplink)");
    plot_waterfall_setPlotYAxisScale(&rb_allocs_up, 0, 65536);
    plot_waterfall_setSpectrogramZAxisScale(&rb_allocs_up, 0, 65536);
#endif

#ifdef GUI_ENABLE_PERFORMANCEPLOTS
    plot_real_init(&p_rb_up);
    plot_real_init(&p_rb_dw);
    plot_real_init(&p_bw_up);
    plot_real_init(&p_bw_dw);
    plot_real_setTitle(&p_rb_up, "Uplink Resource Block Usage");
    plot_real_setTitle(&p_rb_dw, "Downlink Resource Block Usage");
    plot_real_setTitle(&p_bw_up, "Uplink Throughput [kbps]");
    plot_real_setTitle(&p_bw_dw, "Downlink Throughput [kbps]");
    plot_real_setYAxisScale(&p_rb_up, 0, ue_dl.cell.nof_prb);
    plot_real_setYAxisScale(&p_rb_dw, 0, ue_dl.cell.nof_prb);
    plot_real_setYAxisScale(&p_bw_up, 0, 1000*ue_dl.cell.nof_prb);
    plot_real_setYAxisScale(&p_bw_dw, 0, 1000*ue_dl.cell.nof_prb);
#endif

#ifdef GUI_ENABLE_HISTOGRAMPLOT
    for(int hidx=0; hidx<NUM_HIST; hidx++) {
        plot_real_init(&hist[hidx]);
        plot_real_setTitle(&hist[hidx], "RNTI Histogram");
        plot_real_setYAxisScale(&hist[hidx], 0, 100);
    }
#endif

    bzero(falcon_ue_dl.colored_rb_map_dw, sizeof(((falcon_ue_dl_t *)0)->colored_rb_map_dw_bufA));
    bzero(falcon_ue_dl.colored_rb_map_up, sizeof(((falcon_ue_dl_t *)0)->colored_rb_map_up_bufA));

    while(1) {
        sem_wait(&plot_sem);

#ifdef GUI_ENABLE_PERFORMANCEPLOTS
        for (i = 0; i < 1024; i++) {
            tmp_plot[1023-i] = rb_up[(1024+plot_sf_idx-i)%1024];
            tmp_plot2[1023-i] = rb_dw[(1024+plot_sf_idx-i)%1024];
            tmp_plot3[1023-i] = bw_up[(1024+plot_sf_idx-i)%1024];
            tmp_plot4[1023-i] = bw_dw[(1024+plot_sf_idx-i)%1024];
        }
#endif
#ifdef GUI_ENABLE_SPECTROGRAM
        bzero(tmp_plot_wf,12*ue_dl.cell.nof_prb*sizeof(float));
        for (j = 0; j < 14; j++) {
            for (i = 0; i < 12*ue_dl.cell.nof_prb; i++) {
                tmp_plot_wf[i] += 20 * log10f(cabsf(ue_dl.sf_symbols[i+j*(12*ue_dl.cell.nof_prb)]))/14;
            }
        }
#endif

#ifdef GUI_ENABLE_PERFORMANCEPLOTS
        plot_real_setNewData(&p_rb_up, tmp_plot, 1024);
        plot_real_setNewData(&p_rb_dw, tmp_plot2, 1024);
        plot_real_setNewData(&p_bw_up, tmp_plot3, 1024);
        plot_real_setNewData(&p_bw_dw, tmp_plot4, 1024);
#endif
#ifdef GUI_ENABLE_SPECTROGRAM
        plot_waterfall_appendNewData(&poutfft, tmp_plot_wf, 12*(int)ue_dl.cell.nof_prb);
#endif
#ifdef GUI_ENABLE_RBALLOC_PLOT
        if(current_colored_rb_map_dw != NULL)
            plot_waterfall_appendNewData(&rb_allocs_dw, current_colored_rb_map_dw, (int)ue_dl.cell.nof_prb);
        if(current_colored_rb_map_up != NULL)
            plot_waterfall_appendNewData(&rb_allocs_up, current_colored_rb_map_up, (int)ue_dl.cell.nof_prb);
#endif
#ifdef GUI_ENABLE_HISTOGRAMPLOT
        for(int hidx=0; hidx<NUM_HIST; hidx++) {
            for(j = 0; j < 65536; j++) tmp_hist[j] = falcon_ue_dl.rnti_histogram[hidx].rnti_histogram[j];
            plot_real_setNewData(&hist[hidx], tmp_hist, 65536);
        }
#endif

#ifdef GUI_ENABLE_NETSINK
        if (plot_sf_idx == 1) {
            if (prog_args.net_port_signal > 0) {
                srslte_netsink_write(&net_sink_signal, &sf_buffer[srslte_ue_sync_sf_len(&ue_sync)/7],
                        srslte_ue_sync_sf_len(&ue_sync));
            }
        }
#endif

    }

    return NULL;
}

void init_plots() {

    if (sem_init(&plot_sem, 0, 0)) {
        perror("sem_init");
        exit(-1);
    }

    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 0;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    pthread_attr_setschedparam(&attr, &param);
    if (pthread_create(&plot_thread, NULL, plot_thread_run, NULL)) {
        perror("pthread_create");
        exit(-1);
    }
}

