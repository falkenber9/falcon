#include "capture_probe/ArgManager.h"
#include "capture_probe/CaptureProbeCore.h"

#include "falcon/version.h"

#include "falcon/meas/AuxModem.h"
#include "falcon/meas/AuxModemGPS.h"
#include "falcon/meas/TrafficGenerator.h"
#include "falcon/meas/TrafficResultsToFile.h"
#include "falcon/meas/NetsyncSlave.h"
#include "falcon/common/BufferedFileSink.h"
#include "falcon/common/SignalManager.h"

#include "falcon/common/SystemInfo.h"

#include <iostream>
#include <memory>
#include <cstdlib>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv) {
  cout << "FalconCaptureProbe, version: " << falcon_get_version_git() << endl;
  cout << "Copyright (C) 2019 Robert Falkenberg" << endl;
  cout << endl;

  Args args;
  ArgManager::parseArgs(args, argc, argv);

  NetsyncSlave netsync(args.port);
  //netsync.attachExtraStopFlag(&go_exit);
  netsync.launchReceiver();

  AuxModem modem;
  AuxModemGPS gps;
  DummyGPS gpsDummy;
  GPS* gpsRef = &gpsDummy;

  if(!modem.init()) {
    cout << "Could not initialize AuxModem" << endl;
    return EXIT_FAILURE;
  }
  if(!modem.configure()) {
    cout << "Could not configure AuxModem" << endl;
    return EXIT_FAILURE;
  }
  if(!gps.init(&modem)) {
    cout << "Warning: Could not initialize GPS from AuxModem - continue without GPS" << endl;
  }
  else {
    gpsRef = &gps;
  }

  netsync.init(&modem);

  TrafficGenerator trafficGenerator;

#define USE_BUFFERED_SINK
#ifdef USE_BUFFERED_SINK
  BufferedFileSink<cf_t> sink;
  size_t sz = SystemInfo::getAvailableRam() - 512*1024*1024;  //allocate all RAM but 512MB
  cout << "Allocating memory sample buffer of " << sz << " B..." << endl;
  sink.allocate(sz);
#else
  FileSink<cf_t> sink;
#endif

  //attach signal handlers (for CTRL+C)
  SignalGate& signalGate(SignalGate::getInstance());
  signalGate.init();
  signalGate.attach(netsync);

  bool runSuccess = false;
  int nof_runs = 1;
  do {
    CaptureProbeCore core(args);
    core.init(&netsync, &modem, &trafficGenerator, &sink, gpsRef);
    signalGate.attach(core);
    runSuccess = core.run();
    signalGate.detach(core);
    if(args.repeat_pause > 0) {
      sleep(args.repeat_pause);
    }
  } while(runSuccess && (args.nof_runs != nof_runs++));

  cout << "Exit" << endl;
  return runSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}
