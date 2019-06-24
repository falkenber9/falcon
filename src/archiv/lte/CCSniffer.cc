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
#include "CCSniffer.h"

#include "srslte/srslte.h"
#include "srslte/common/log.h"
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

using namespace srslte;

CCSniffer* CCSniffer::instance = NULL;

CCSniffer::CCSniffer() : started(false) {
}

CCSniffer::~CCSniffer() {
  for (uint32_t i = 0; i < phy_log.size(); i++) {
    if (phy_log[i]) {
      delete(phy_log[i]);
    }
  }
}

CCSniffer* CCSniffer::get_instance() {
  if(instance == NULL) {
    instance = new CCSniffer();
  }
  return instance;
}

bool CCSniffer::init(srsue::all_args_t* args_) {
  args = args_;

  int nof_phy_threads = args->expert.phy.nof_phy_threads;
  if (nof_phy_threads > 3) {
    nof_phy_threads = 3;
  }

  if (!args->log.filename.compare("stdout")) {
    logger = &logger_stdout;
  } else {
    logger_file.init(args->log.filename, args->log.file_max_size);
    logger_file.log("\n\n");
    logger_file.log(get_build_string().c_str());
    logger = &logger_file;
  }

  rf_log.init("RF  ", logger);
  // Create array of pointers to phy_logs
  for (int i=0;i<nof_phy_threads;i++) {
    srslte::log_filter *mylog = new srslte::log_filter;
    char tmp[16];
    sprintf(tmp, "PHY%d",i);
    mylog->init(tmp, logger, true);
    phy_log.push_back(mylog);
  }

  // Init logs
  rf_log.set_level(srslte::LOG_LEVEL_INFO);
  rf_log.info("Starting UE\n");
  for (int i=0;i<nof_phy_threads;i++) {
    ((srslte::log_filter*) phy_log[i])->set_level(level(args->log.phy_level));
  }

  /* here we add a log layer to handle logging from the phy library*/
  if (level(args->log.phy_lib_level) != LOG_LEVEL_NONE) {
    srslte::log_filter *lib_log = new srslte::log_filter;
    char tmp[16];
    sprintf(tmp, "PHY_LIB");
    lib_log->init(tmp, logger, true);
    phy_log.push_back(lib_log);
    ((srslte::log_filter*) phy_log[nof_phy_threads])->set_level(level(args->log.phy_lib_level));
  } else {
    phy_log.push_back(NULL);
  }

  for (int i=0;i<nof_phy_threads + 1;i++) {
    if (phy_log[i]) {
      ((srslte::log_filter*) phy_log[i])->set_hex_limit(args->log.phy_hex_limit);
    }
  }

  if(args->trace.enable) {
    phy.start_trace();
    radio.start_trace();
  }

  // Init layers

  // PHY inits in background, start before radio
  args->expert.phy.nof_rx_ant = args->rf.nof_rx_ant;
  //phy.init(&radio, &mac, &rrc, phy_log, &args->expert.phy);
  phy.init(&radio, NULL, NULL, phy_log, &args->expert.phy);

  /* Start Radio */
  char *dev_name = NULL;
  if (args->rf.device_name.compare("auto")) {
    dev_name = (char*) args->rf.device_name.c_str();
  }

  char *dev_args = NULL;
  if (args->rf.device_args.compare("auto")) {
    dev_args = (char*) args->rf.device_args.c_str();
  }

  printf("Opening RF device with %d RX antennas...\n", args->rf.nof_rx_ant);
  if(!radio.init_multi(args->rf.nof_rx_ant, dev_args, dev_name)) {
    printf("Failed to find device %s with args %s\n",
           args->rf.device_name.c_str(), args->rf.device_args.c_str());
    return false;
  }

  // Set RF options
  if (args->rf.time_adv_nsamples.compare("auto")) {
    int t = atoi(args->rf.time_adv_nsamples.c_str());
    radio.set_tx_adv(abs(t));
    radio.set_tx_adv_neg(t<0);
  }
  if (args->rf.burst_preamble.compare("auto")) {
    radio.set_burst_preamble(atof(args->rf.burst_preamble.c_str()));
  }
  if (args->rf.continuous_tx.compare("auto")) {
    printf("set continuous %s\n", args->rf.continuous_tx.c_str());
    radio.set_continuous_tx(args->rf.continuous_tx.compare("yes")?false:true);
  }

  // Set PHY options
  if (args->rf.tx_gain > 0) {
    args->expert.phy.ul_pwr_ctrl_en = false;
  } else {
    args->expert.phy.ul_pwr_ctrl_en = true;
  }

  if (args->rf.rx_gain < 0) {
    radio.start_agc(false);
  } else {
    radio.set_rx_gain(args->rf.rx_gain);
  }
  if (args->rf.tx_gain > 0) {
    radio.set_tx_gain(args->rf.tx_gain);
  } else {
    radio.set_tx_gain(args->rf.rx_gain);
    std::cout << std::endl <<
                "Warning: TX gain was not set. " <<
                "Using open-loop power control (not working properly)" << std::endl << std::endl;
  }

  radio.register_error_handler(rf_msg);
  radio.set_freq_offset(args->rf.freq_offset);

  // Get current band from provided EARFCN
  args->rrc.supported_bands[0] = srslte_band_get_band(args->rf.dl_earfcn);
  args->rrc.nof_supported_bands = 1;
  args->rrc.ue_category = atoi(args->ue_category_str.c_str());

  // Currently EARFCN list is set to only one frequency as indicated in ue.conf
  std::vector<uint32_t> earfcn_list;
  earfcn_list.push_back(args->rf.dl_earfcn);
  phy.set_earfcn(earfcn_list);

  if (args->rf.dl_freq > 0 && args->rf.ul_freq > 0) {
    phy.force_freq(args->rf.dl_freq, args->rf.ul_freq);
  }

  printf("Waiting PHY to initialize...\n");
  phy.wait_initialize();
  phy.configure_ul_params();

  // Enable AGC once PHY is initialized
  if (args->rf.rx_gain < 0) {
    phy.set_agc_enable(true);
  }

  printf("...\n");

  started = true;
  return true;
}

void CCSniffer::pregenerate_signals(bool enable) {
  phy.enable_pregen_signals(enable);
}

void CCSniffer::stop()
{
  if(started)
  {

    // Caution here order of stop is very important to avoid locks


    // PHY must be stopped before radio otherwise it will lock on rf_recv()
    phy.stop();
    radio.stop();

    usleep(1e5);
    if(args->trace.enable) {
      phy.write_trace(args->trace.phy_filename);
      radio.write_trace(args->trace.radio_filename);
    }
    started = false;
  }
}

bool CCSniffer::switch_on() {
  printf("Attach not implemented yet\n");
  //return nas.attach_request();
  return false;
}

bool CCSniffer::switch_off() {
  // generate detach request
  //nas.detach_request();

  return true;
}

bool CCSniffer::is_attached() {
  return false;
}

void CCSniffer::start_plot() {
  phy.start_plot();
}

void CCSniffer::print_mbms() {
  // Nothing
}

bool CCSniffer::mbms_service_start(uint32_t serv, uint32_t port) {
  return false;
}

void CCSniffer::print_pool() {
  byte_buffer_pool::get_instance()->print_all_buffers();
}

void CCSniffer::rf_msg(srslte_rf_error_t error) {
  ue_base *ue = CCSniffer::get_instance();
  ue->handle_rf_msg(error);
  if (error.type == srslte_rf_error_t::SRSLTE_RF_ERROR_OVERFLOW) {
    ue->radio_overflow();
  } else
  if (error.type == srslte_rf_error_t::SRSLTE_RF_ERROR_RX) {
    ue->stop();
    ue->cleanup();
    exit(-1);
  }
}

bool CCSniffer::get_metrics(srsue::ue_metrics_t &m)
{
  bzero(&m, sizeof(srsue::ue_metrics_t));
  m.rf = rf_metrics;
  bzero(&rf_metrics, sizeof(srsue::rf_metrics_t));
  rf_metrics.rf_error = false; // Reset error flag

  phy.get_metrics(m.phy);
  return true;
}

void CCSniffer::radio_overflow() {
  phy.radio_overflow();
}


