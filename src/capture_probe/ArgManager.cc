#include "falcon/common/Settings.h"
#include "ArgManager.h"

#include "srslte/srslte.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>

#define ENABLE_AGC_DEFAULT

using namespace std;

void ArgManager::defaultArgs(Args& args) {
  args.cpu_affinity = -1;
  args.disable_cfo = false;
  //args.force_N_id_2 = -1; // Pick the best
  //args.nof_prb = 25;
  args.rf_args = "";
  args.rf_nof_rx_ant = 1;
  args.rf_freq = 0.0;
#ifdef ENABLE_AGC_DEFAULT
  args.rf_gain = -1.0;
#else
  args.rf_gain = 50.0;
#endif
  args.decimate = 0;

  args.nof_runs = 1;
  args.repeat_pause = 0;
  args.backoff = 0;

  args.client_mode = false;
  args.port = DEFAULT_NETSYNC_PORT;
  args.output_file_base_name = "";
  args.nof_subframes = DEFAULT_NOF_SUBFRAMES_TO_CAPTURE;
  args.probing_delay = DEFAULT_PROBING_DELAY_MS;
  args.direction = DEFAULT_PROBING_DIRECTION;
  args.payload_size = DEFAULT_PROBING_PAYLOAD_SIZE;
  args.url = "";
  args.tx_power_sample_interval = DEFAULT_TX_POWER_SAMPLING_INTERVAL_US;
}

void ArgManager::usage(Args& args, const std::string& prog) {
  printf("Usage: %s [aAbcCDfgIlLnNopvWXyZ] [-c | -o output_file_base_name]\n", prog.c_str());
  printf("\t-a RF args [Default %s]\n", args.rf_args.c_str());
  printf("\t-A Number of RX antennas [Default %d]\n", args.rf_nof_rx_ant);
  printf("\t-b Backoff in integer multiples of probing_delay + probing timeout before each run (default: %d)\n", args.backoff);
  printf("\t-c Client mode, controlled by FalconCaptureWarden\n");
  printf("\t-C Disable CFO correction [Default %s]\n", args.disable_cfo?"Disabled":"Enabled");
  printf("\t-f Override rf_frequency [Default frequency from aux modem]\n");
#ifdef ENABLE_AGC_DEFAULT
  printf("\t-g RF fix RX gain [Default AGC]\n");
#else
  printf("\t-g Set RX gain [Default %.1f dB]\n", args.rf_gain);
#endif
  //printf("\t-l Force N_id_2 [Default best]\n");
  printf("\t-n nof_subframes [Default %d], 0=unlimited\n", args.nof_subframes);
  //printf("\t-p nof_prb [Default %d]\n", args.nof_prb);
  printf("\t-v [set srslte_verbose to debug, default none]\n");
  printf("\t-y Set the cpu affinity mask [Default %d]\n",args.cpu_affinity);
  printf("\t-Z Set decimation value [Default %d]\n",args.decimate);
  // external modem probing
  printf("\t-N Number of runs (default: %d, 0=unlimited)\n",args.nof_runs);
  printf("\t-I Pause between repetitions in sec (default: %d)\n",args.repeat_pause);
  printf("\t-L Set Payload size in bytes (default: %ld)\n",args.payload_size);
  printf("\t-X Set direction (upload=0, download=1) (default: %d)\n",args.direction);
  printf("\t-W Set target URL, Default:\n\t\tUL: %s\n\t\tDL: %s\n", DEFAULT_PROBING_URL_UPLINK, DEFAULT_PROBING_URL_DOWNLINK);
  printf("\t-D Probing delay [ms] (default: %d)\n",args.probing_delay);
  printf("\t-T TX power sampling interval [us] (default: %d), 0=disabled\n",args.tx_power_sample_interval);
}

void ArgManager::parseArgs(Args& args, int argc, char **argv) {
  int opt;
  defaultArgs(args);
  while ((opt = getopt(argc, argv, "aAbcCDfgIlLnNopTvWXyZ")) != -1) {
    switch (opt) {
      //case 'p':
      //  args.nof_prb = atoi(argv[optind]);
      //  break;
      case 'o':
        args.output_file_base_name = argv[optind];
        break;
      case 'a':
        args.rf_args = argv[optind];
        break;
      case 'A':
        args.rf_nof_rx_ant = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'b':
        args.backoff = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'g':
        args.rf_gain = strtod(argv[optind], nullptr);
        break;
      case 'c':
        args.client_mode = true;
        break;
      case 'C':
        args.disable_cfo = true;
        break;
      case 'f':
        args.rf_freq = strtod(argv[optind], nullptr);
        break;
      case 'n':
        args.nof_subframes = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
        //case 'l':
        //  args.force_N_id_2 = atoi(argv[optind]);
        //  break;
      case 'v':
        srslte_verbose++;
        break;
      case 'Z':
        args.decimate = atoi(argv[optind]);
        break;
      case 'y':
        args.cpu_affinity = atoi(argv[optind]);
        break;
        // external modem probing
      case 'W':
        args.url = argv[optind];
        break;
      case 'N':
        args.nof_runs = atoi(argv[optind]);
        break;
      case 'I':
        args.repeat_pause = atoi(argv[optind]);
        break;
      case 'L':
        args.payload_size = atoi(argv[optind]);
        break;
      case 'X':
        args.direction = atoi(argv[optind]);
        if(args.direction > 1) {
          cerr << "Invalid direction: " << args.direction << endl;
          usage(args, argv[0]);
          exit(-1);
        }
        break;
      case 'D':
        args.probing_delay = atoi(argv[optind]);
        break;
      case 'T':
        args.tx_power_sample_interval = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      default:
        usage(args, argv[0]);
        exit(-1);
    }
  }
  if (!args.client_mode && args.output_file_base_name == "") {
    cerr << "Neither client-mode '-c' selected, nor output_file_base_name '-o' provided" << endl;
    usage(args, argv[0]);
    exit(-1);
  }
  if (args.url.length() == 0) {
    switch (args.direction) {
      case DIRECTION_UPLINK:
        args.url = DEFAULT_PROBING_URL_UPLINK;
        break;
      case DIRECTION_DOWNLINK:
        args.url = DEFAULT_PROBING_URL_DOWNLINK;
        break;
      default:
        cerr << "Invalid direction" << endl;
        usage(args, argv[0]);
        exit(-1);
    }
  }
}
