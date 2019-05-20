#pragma once

#include "falcon/CCSnifferInterfaces.h"
#include "falcon/definitions.h"
#include <boost/thread.hpp>

//#define SPECTROGRAM_LINE_COUNT 100
//#define SPECTROGRAM_LINE_WIDTH 100
#define SPECTROGRAM_INTERVAL_US 10000

class ScanThread :
    public Provider<ScanLineLegacy> {
public:
  ScanThread() :
    Provider (),
    cancel(false),
    initialized(false),
    theThread(NULL)
  {}
  virtual ~ScanThread();

  int spectrogram_line_width = 50;

  void init();
  void start();
  void stop();
  bool isInitialized();
private:
  void run();
  volatile bool cancel;
  bool initialized;
  boost::thread* theThread;
};
