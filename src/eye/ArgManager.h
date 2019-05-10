#pragma once

#include <stdint.h>
#include <string>

struct Args {
  uint32_t nof_subframes;
  int cpu_affinity;
  bool disable_plots;
  bool disable_cfo;
  uint32_t time_offset;
  int force_N_id_2;
  std::string input_file_name;
  std::string dci_file_name;
  std::string stats_file_name;
  //char *rnti_list_file;
  //char *rnti_list_file_out;
  int file_offset_time;
  double file_offset_freq;
  uint32_t file_nof_prb;
  uint32_t file_nof_ports;
  uint32_t file_cell_id;
  std::string rf_args;
  uint32_t rf_nof_rx_ant;
  double rf_freq;
  double rf_gain;
  //int net_port;
  //char *net_address;
  //int net_port_signal;
  //char *net_address_signal;
  int decimate;
};

class ArgManager {
public:
  static void defaultArgs(Args& args);
  static void usage(Args& args, const std::string& prog);
  static void parseArgs(Args& args, int argc, char **argv);
private:
  ArgManager() = delete;  // static only
};
