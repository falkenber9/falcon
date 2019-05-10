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

    for (unsigned int col = 0; col < SPECTROGRAM_LINE_WIDTH; ++col) {
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
