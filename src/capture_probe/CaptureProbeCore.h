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
