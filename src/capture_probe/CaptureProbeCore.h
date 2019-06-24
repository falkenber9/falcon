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

#include "ArgManager.h"
#include "falcon/meas/AuxModem.h"
#include "falcon/meas/GPS.h"
#include "falcon/meas/NetsyncSlave.h"
#include "falcon/meas/TrafficGenerator.h"
#include "falcon/meas/TrafficResultsToFile.h"
#include "falcon/meas/Cancelable.h"
#include "falcon/common/FileSink.h"
#include "falcon/common/SignalManager.h"

#include "falcon/common/Settings.h"

#include "srslte/srslte.h"

// include C-only headers
#ifdef __cplusplus
    extern "C" {
#endif

#include "srslte/phy/common/phy_common.h"

#define ENABLE_AGC_DEFAULT

#include "srslte/phy/rf/rf.h"
#include "srslte/phy/rf/rf_utils.h"

#ifdef __cplusplus
}
#undef I // Fix complex.h #define I nastiness when using C++
#endif

//#define STDOUT_COMPACT
#define PRINT_CHANGE_SCHEDULING
//#define CORRECT_SAMPLE_OFFSET

class CaptureProbeCore : public SignalHandler, public Cancelable {
public:
  //construction
  CaptureProbeCore(Args& args);
  CaptureProbeCore(const CaptureProbeCore&) = delete; //prevent copy
  CaptureProbeCore& operator=(const CaptureProbeCore&) = delete; //prevent copy
  virtual ~CaptureProbeCore() override;

  //setup
  void init(NetsyncSlave* netsync,
            AuxModem* modem,
            TrafficGenerator* trafficGen,
            FileSink<cf_t>* sink, GPS* gps);

  //incoming interfaces
  void cancel() override;

  //other public methods
  bool run();
private:
  //incoming interfaces
  void handleSignal() override;

  //other methods
  static void signalInterruptHandler(int sigNo);
  std::string outputFileBaseName();

  //outgoing interfaces
  NetsyncSlave* netsync;
  AuxModem* modem;
  GPS* gps;
  TrafficGenerator* trafficGen;
  FileSink<cf_t>* sink;

  //internal variables
  bool go_exit;
  bool critical;
  Args& args;
  uint8_t *data[SRSLTE_MAX_CODEWORDS];

};
