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
  // No pointer members! Avoid shallow copies

  uint32_t nof_subframes;
  int cpu_affinity;
  bool enable_ASCII_PRB_plot;
  bool enable_ASCII_power_plot;
  bool disable_cfo;
  uint32_t time_offset;
  int force_N_id_2;
  std::string input_file_name = "";
  std::string dci_file_name = "";
  std::string stats_file_name = "";
  int file_offset_time;
  double file_offset_freq;
  uint32_t file_nof_prb;
  uint32_t file_nof_ports;
  uint32_t file_cell_id;
  bool file_wrap;
  std::string rf_args;
  uint32_t rf_nof_rx_ant;
  double rf_freq;
  double rf_gain;
  int decimate;
  int nof_subframe_workers;

  // other config args
  uint32_t dci_format_split_update_interval_ms;
  double dci_format_split_ratio;
  bool skip_secondary_meta_formats;
  bool enable_shortcut_discovery;
  uint32_t rnti_histogram_threshold;
};

class ArgManager {
public:
  static void defaultArgs(Args& args);
  static void usage(Args& args, const std::string& prog);
  static void parseArgs(Args& args, int argc, char **argv);
private:
  ArgManager() = delete;  // static only
};
