
#pragma once

#include <stdint.h>
#include <string>
#include <pthread.h>

#include "falcon/CCSnifferInterfaces.h"

#include "srsue/hdr/ue_base.h"
#include "srslte/radio/radio_multi.h"
#include "srsue/hdr/phy/phy.h"

#include "srslte/common/logger.h"
#include "srslte/common/logger_file.h"
#include "srslte/common/log_filter.h"


class CCSniffer :
    public srsue::ue_base,
    public CCSnifferInterface,
    public SnifferConfigInterface,
    public SnifferVisualizationInterface {
public:
  CCSniffer();
  static CCSniffer* get_instance();

  bool init(srsue::all_args_t *args_);
  void stop();
  bool switch_on();
  bool switch_off();
  bool is_attached();
  void start_plot();
  void print_mbms();
  bool mbms_service_start(uint32_t serv, uint32_t port);

  void print_pool();

  static void rf_msg(srslte_rf_error_t error);

  // UE metrics interface
  bool get_metrics(srsue::ue_metrics_t &m);

  void pregenerate_signals(bool enable);

  void radio_overflow();
private:
  virtual ~CCSniffer();

  static CCSniffer* instance;
  pthread_mutex_t ue_instance_mutex = PTHREAD_MUTEX_INITIALIZER;

  srslte::radio_multi radio;
  srsue::phy phy;

  srslte::logger_stdout logger_stdout;
  srslte::logger_file   logger_file;
  srslte::logger        *logger;

  std::vector<srslte::log_filter*>  phy_log;

  srsue::all_args_t       *args;
  bool                    started;

  bool check_srslte_version();

};
