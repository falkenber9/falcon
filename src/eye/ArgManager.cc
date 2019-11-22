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
#include "falcon/common/Settings.h"
#include "ArgManager.h"

#include "srslte/srslte.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>

#define ENABLE_AGC_DEFAULT

using namespace std;

void ArgManager::defaultArgs(Args& args) {
  args.nof_subframes = DEFAULT_NOF_SUBFRAMES_TO_SHOW;
  args.cpu_affinity = -1;
  args.enable_ASCII_PRB_plot = true;
  args.enable_ASCII_power_plot = false;
  args.disable_cfo = false;
  args.time_offset = 0;
  args.force_N_id_2 = -1; // Pick the best
  args.input_file_name = "";
  args.dci_file_name = "";
  args.stats_file_name = "";
  //args.rnti_list_file = "";
  //args.rnti_list_file_out = "";
  args.file_offset_time = 0;
  args.file_offset_freq = 0;
  args.file_nof_prb = DEFAULT_NOF_PRB;
  args.file_nof_ports = DEFAULT_NOF_PORTS;
  args.file_cell_id = 0;
  args.file_wrap = false;
  args.rf_args = "";
  args.rf_freq = -1.0;
  args.rf_nof_rx_ant = DEFAULT_NOF_RX_ANT;
#ifdef ENABLE_AGC_DEFAULT
  args.rf_gain = -1.0;
#else
  args.rf_gain = 50.0;
#endif
  //args.net_port = -1;
  //args.net_address = "127.0.0.1";
  //args.net_port_signal = -1;
  //args.net_address_signal = "127.0.0.1";
  args.decimate = 0;

  // other args
  args.dci_format_split_update_interval_ms = DEFAULT_DCI_FORMAT_SPLIT_UPDATE_INTERVAL_MS;
  args.dci_format_split_ratio = DEFAULT_DCI_FORMAT_SPLIT_RATIO;
  args.skip_secondary_meta_formats = false;
  args.enable_shortcut_discovery = true;
}

void ArgManager::usage(Args& args, const std::string& prog) {
  printf("Usage: %s [aAcCdfgHilnoOpPrRsStTvwyY] -f rx_frequency (in Hz) | -i input_file\n", prog.c_str());
#ifndef DISABLE_RF
  printf("\t-a RF args [Default %s]\n", args.rf_args.c_str());
  printf("\t-A Number of RX antennas [Default %d]\n", args.rf_nof_rx_ant);
#ifdef ENABLE_AGC_DEFAULT
  printf("\t-g RF fix RX gain [Default AGC]\n");
#else
  printf("\t-g Set RX gain [Default %.1f dB]\n", args.rf_gain);
#endif
#else
  printf("\t   RF is disabled.\n");
#endif
  printf("\t-H disable shortcut discovery (stick to histogram and random access)\n");
  printf("\t-i input_file [Default use RF board]\n");
  printf("\t-w wrap input_file after reading all samples\n");
  printf("\t-D output filename for DCI [default stdout]\n");
  printf("\t-E output filename for statistics [default stdout]\n");
  printf("\t-o offset frequency correction (in Hz) for input file [Default %.1f Hz]\n", args.file_offset_freq);
  printf("\t-O offset samples for input file [Default %d]\n", args.file_offset_time);
  printf("\t-p nof_prb for input file [Default %d]\n", args.file_nof_prb);
  printf("\t-P nof_ports for input file [Default %d]\n", args.file_nof_ports);
  printf("\t-c cell_id for input file [Default %d]\n", args.file_cell_id);
  printf("\t-l Force N_id_2 [Default best]\n");
  printf("\t-C Disable CFO correction [Default %s]\n", args.disable_cfo ? "Disabled" : "Enabled");
  printf("\t-t Add time offset [Default %d]\n", args.time_offset);
  printf("\t-r disable ASCII PRB plots [Default enabled]\n");
  printf("\t-R enable ASCII Power plot [Default disabled]\n");
  printf("\t-y set the cpu affinity mask [Default %d]\n", args.cpu_affinity);
  printf("\t-Y set the decimate value [Default %d]\n", args.decimate);
  printf("\t-n nof_subframes [Default %d]\n", args.nof_subframes);
  //printf("\t-s remote UDP port to send input signal (-1 does nothing with it) [Default %d]\n", args.net_port_signal);
  //printf("\t-S remote UDP address to send input signal [Default %s]\n", args.net_address_signal);
  //printf("\t-u remote TCP port to send data (-1 does nothing with it) [Default %d]\n", args.net_port);
  //printf("\t-U remote TCP address to send data [Default %s]\n", args.net_address);
  printf("\t-s skip decoding of secondary (less frequent) DCI formats\n");
  printf("\t-S split ratio for primary/secondary DCI formats [0.0..1.0, Default %f]\n", args.dci_format_split_ratio);
  printf("\t-T interval to perform dci format split [Default %d ms]\n", args.dci_format_split_update_interval_ms);
  printf("\t-v [set srslte_verbose to debug, default none]\n");
  //printf("\t-z filename of the output reporting one int per rnti (tot length 64k entries)\n");
  //printf("\t-Z filename of the input reporting one int per rnti (tot length 64k entries)\n");
}

void ArgManager::parseArgs(Args& args, int argc, char **argv) {
  int opt;
  defaultArgs(args);
  while ((opt = getopt(argc, argv, "aAcCDEfgHilnpPrRsStTvwyY")) != -1) {
    switch (opt) {
      case 'a':
        args.rf_args = argv[optind];
        break;
      case 'A':
        args.rf_nof_rx_ant = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'g':
        args.rf_gain = strtod(argv[optind], nullptr);
        break;
      case 'H':
        args.enable_shortcut_discovery = false;
        break;
      case 'i':
        args.input_file_name = argv[optind];
        break;
      case 'w':
        args.file_wrap = true;
        break;
      case 'D':
        args.dci_file_name = argv[optind];
        break;
      case 'E':
        args.stats_file_name = argv[optind];
        break;
      case 'o':
        args.file_offset_freq = strtod(argv[optind], nullptr);
        break;
      case 'O':
        args.file_offset_time = atoi(argv[optind]);
        break;
      case 'p':
        args.file_nof_prb = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'P':
        args.file_nof_ports = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'c':
        args.file_cell_id = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'l':
        args.force_N_id_2 = atoi(argv[optind]);
        break;
      case 'C':
        args.disable_cfo = true;
        break;
      case 's':
        args.skip_secondary_meta_formats = true;
        break;
      case 'S':
        args.dci_format_split_ratio = strtod(argv[optind], nullptr);
        break;
      case 't':
        args.time_offset = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'T':
        args.dci_format_split_update_interval_ms = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'r':
        args.enable_ASCII_PRB_plot = false;
        break;
      case 'R':
        args.enable_ASCII_power_plot = true;
        break;
      case 'y':
        args.cpu_affinity = atoi(argv[optind]);
        break;
      case 'Y':
        args.decimate = atoi(argv[optind]);
        break;
      case 'n':
        args.nof_subframes = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'v':
        srslte_verbose++;
        break;
      case 'f':
        args.rf_freq = strtod(argv[optind], nullptr);
        break;
      default:
        usage(args, argv[0]);
        exit(-1);
    }
  }

  if (args.rf_freq < 0 && args.input_file_name == "") {
    usage(args, argv[0]);
    exit(-1);
  }
}
