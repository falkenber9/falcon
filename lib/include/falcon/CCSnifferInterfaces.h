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

#include <vector>
#include <algorithm>
#include <stdint.h>
#include <vector>

#include "definitions.h"
#include "falcon/util/RNTIManager.h"

class CCSnifferInterface {

};

class SnifferConfigInterface {

};

template <typename PushDataType>
class Subscriber {
public:
  virtual void update() = 0;
  virtual void push(const PushDataType*) = 0;
  virtual void notifyDetach() = 0;  // Forced detach
};

template <typename PushDataType>
class Provider {

private:
  std::vector<Subscriber<PushDataType>*> subscriber;

public:
  virtual ~Provider() {
    for (typename std::vector<Subscriber<PushDataType>*>::size_type i = 0; i < subscriber.size(); i++)
      subscriber[i]->notifyDetach();
  }

  void subscribe(Subscriber<PushDataType>* s) {
    subscriber.push_back(s);
  }

  void unsubscribe(Subscriber<PushDataType>* s) {
     subscriber.erase(std::remove(subscriber.begin(), subscriber.end(), s), subscriber.end());
  }

  void notifySubscribers() {
    for (typename std::vector<Subscriber<PushDataType>*>::size_type i = 0; i < subscriber.size(); i++)
      subscriber[i]->update();
  }

  void pushToSubscribers(const PushDataType* data) {
    for (typename std::vector<Subscriber<PushDataType>*>::size_type i = 0; i < subscriber.size(); i++)
      subscriber[i]->push(data);
  }

protected:

};

typedef enum {
  SCAN_LINE_UPLINK        = 0,
  SCAN_LINE_DOWNLINK      = 1,
  SCAN_LINE_SPECTRUM      = 2,
  SCAN_LINE_SPECTRUM_DIFF = 3,
  SCAN_LINE_RNTI_HIST     = 4,
  SCAN_LINE_PERF_PLOT_A   = 5,
  SCAN_LINE_PERF_PLOT_B   = 6
} ScanLineType_t;

class ScanLineLegacy{
public:
  virtual ~ScanLineLegacy(){}
  ScanLineType_t type;
  uint16_t linebuf[SPECTROGRAM_LINE_WIDTH];
  std::vector<uint32_t> rnti_hist;
  std::vector<rnti_manager_active_set_t> rnti_active_set;

  uint32_t sf_idx;
  uint32_t mcs_idx;
  int      mcs_tbs;
  uint32_t l_prb;
  uint16_t sfn;
};

class ScanLine {
public:
  ScanLine(float* line, float nItems) : line(line), nItems(nItems) { }
  ScanLine(const ScanLine& ) = delete;  //prevent copy
  ScanLine& operator=(const ScanLine&) = delete;  //prevent copy
  ~ScanLine() { }
private:
  float* line;
  float nItems;
};

//class SnifferVisualizationInterface : public Provider {
//  // to be filled with methods...
//  virtual LineData* getUplinkLine() { return NULL; };
//  virtual LineData* getDownlinkLine() { return NULL; };
//};
