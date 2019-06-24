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

#include "falcon/CCSnifferInterfaces.h"
#include "falcon/definitions.h"
#include <boost/thread.hpp>
//#include "cni_cc_decoder.h"


//#define SPECTROGRAM_LINE_COUNT 100
//#define SPECTROGRAM_LINE_WIDTH 100
#define SPECTROGRAM_INTERVAL_US 10000
/*
typedef enum {
  SCAN_LINE_UPLINK = 0,
  SCAN_LINE_DOWNLINK
} ScanLineType_t;

class ScanLine : public LineData, public PushData {
public:
  ScanLineType_t type;
  float linebuf[SPECTROGRAM_LINE_WIDTH];
};*/

class DecoderThread :
    public Provider<ScanLineLegacy> {
public:
  DecoderThread() :
    Provider<ScanLineLegacy>(),
    cancel(false),
    initialized(false),
    theThread(NULL)
  {}
  virtual ~DecoderThread();

  void init();
  void start(int width);
  void stop();
  bool isInitialized();
  int scanline_width = 50;
  void test();

private: 
  void run();
  volatile bool cancel;
  bool initialized;
  boost::thread* theThread;
};

