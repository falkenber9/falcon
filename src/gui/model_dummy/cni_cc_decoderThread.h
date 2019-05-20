 
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

