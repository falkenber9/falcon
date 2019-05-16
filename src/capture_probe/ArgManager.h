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
