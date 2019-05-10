#pragma once

#include "ArgManager.h"
//#include "falcon/CCSnifferInterfaces.h"
#include "falcon/common/SignalManager.h"
#include "falcon/util/RNTIManager.h"

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


class EyeCore : public SignalHandler {
public:
  EyeCore(Args& args);
  EyeCore(const EyeCore&) = delete; //prevent copy
  EyeCore& operator=(const EyeCore&) = delete; //prevent copy
  virtual ~EyeCore() override;

  //other public methods
  bool run();

private:
  //incoming interfaces
  void handleSignal() override;

  //internal variables
  bool go_exit;
  Args& args;
  uint8_t *pch_payload_buffers[SRSLTE_MAX_CODEWORDS];
  enum receiver_state { DECODE_MIB, DECODE_PDSCH} state;
//  Provider<ScanLine> uplinkAllocProvider;
//  Provider<ScanLine> downlinkAllocProvider;
//  Provider<ScanLine> downlinkSpectrumProvider;
//  //Provider<ScanLine> downlinkSpectrumDiffProvider;
//  Provider<uint32_t> histogramProvider;
};
