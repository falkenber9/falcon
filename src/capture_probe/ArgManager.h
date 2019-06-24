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
#pragma once

#include <stdint.h>
#include <string>

struct Args {
  // receiver params
  int cpu_affinity;
  bool disable_cfo;
  //int force_N_id_2;
  //uint32_t nof_prb;
  std::string rf_args;
  uint32_t rf_nof_rx_ant;
  double rf_freq;
  double rf_gain;
  int decimate;
  // probing params

  // local params
  int nof_runs;
  int repeat_pause;
  uint32_t backoff;
  // local params for client mode
  bool client_mode;
  bool no_auxmodem;
  uint16_t port;
  // local/remote params
  std::string output_file_base_name;
  uint32_t nof_subframes;
  uint32_t probing_delay;
  uint32_t direction;
  size_t payload_size;
  std::string url;
  uint32_t tx_power_sample_interval;
};

class ArgManager {
public:
  static void defaultArgs(Args& args);
  static void usage(Args& args, const std::string& prog);
  static void parseArgs(Args& args, int argc, char **argv);
private:
  ArgManager() = delete;  // static only
};
