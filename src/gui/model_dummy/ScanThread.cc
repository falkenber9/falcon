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
#include "ScanThread.h"

#include <unistd.h>
#include <iostream>


void ScanThread::init() {
  // setup dependencies of this instance
  initialized = true;
}

void ScanThread::start() {
  if(!isInitialized()) {
    std::cerr << "Error in " << __func__ << ": not initialized." << std::endl;
    return;
  }
  if(theThread != NULL) {
    std::cerr << "Thread already running!" << std::endl;
  }
  theThread = new boost::thread(boost::bind(&ScanThread::run, this));
}

void ScanThread::run() {
  ScanLineLegacy line_ul;
  ScanLineLegacy line_dl;


  line_ul.type        = SCAN_LINE_UPLINK;
  line_dl.type        = SCAN_LINE_DOWNLINK;


  while(!cancel) {
    usleep(SPECTROGRAM_INTERVAL_US);

    for (unsigned int col = 0; col < spectrogram_line_width; ++col) {
      line_ul.linebuf[col]= static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/100000));
      line_dl.linebuf[col]= static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/100000));
    }
    pushToSubscribers(&line_ul);
    pushToSubscribers(&line_dl);
  }
}

void ScanThread::stop() {
  cancel = true;
  if(theThread != NULL) {
    theThread->join();
    delete theThread;
    theThread = NULL;
  }
}

bool ScanThread::isInitialized() {
  return initialized;
}

ScanThread::~ScanThread() {
  stop();
}
