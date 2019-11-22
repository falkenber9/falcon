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
//#include "falcon/CCSnifferInterfaces.h"
#include "falcon/common/SignalManager.h"
#include "falcon/util/RNTIManager.h"
#include "phy/Phy.h"

//#include "srslte/srslte.h"

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


class EyeCore : public SignalHandler {
public:
  EyeCore(const Args& args);
  EyeCore(const EyeCore&) = delete; //prevent copy
  EyeCore& operator=(const EyeCore&) = delete; //prevent copy
  virtual ~EyeCore() override;

  //other public methods
  bool run();
  void stop();
  RNTIManager &getRNTIManager();

  //upper layer interfaces
  void setDCIConsumer(std::shared_ptr<SubframeInfoConsumer> consumer);
  void resetDCIConsumer();

private:
  //incoming interfaces
  void handleSignal() override;

  //internal variables
  bool go_exit;
  Args args;
  enum receiver_state { DECODE_MIB, DECODE_PDSCH} state;
  Phy* phy;
//  Provider<ScanLine> uplinkAllocProvider;
//  Provider<ScanLine> downlinkAllocProvider;
//  Provider<ScanLine> downlinkSpectrumProvider;
//  //Provider<ScanLine> downlinkSpectrumDiffProvider;
//  Provider<uint32_t> histogramProvider;
};
