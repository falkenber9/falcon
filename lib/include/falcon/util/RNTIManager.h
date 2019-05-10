#pragma once

#include <vector>
#include <list>
#include <string.h>
#include <strings.h>
#include <string>

#include "rnti_manager_c.h"

#include "Histogram.h"
#include "Interval.h"

// DCI minimum average llr for accepting DCI for blind decoding
//#define DCI_MINIMUM_AVG_LLR_BOUND 0.5     //0.5

// RRC Inactivity Timer
#define RRC_INACTIVITY_TIMER_MS 10000   // Range 0..60000, default 10000

// RNTI Threshold
#define RNTI_HISTOGRAM_THRESHOLD 5

// RNTI histogram and circular buffer
#define RNTI_HISTOGRAM_ELEMENT_COUNT 65536

#define RNTI_HISTORY_DEPTH_MSEC 200  //200
#define RNTI_PER_SUBFRAME (304/5)
#define RNTI_HISTORY_DEPTH (RNTI_HISTORY_DEPTH_MSEC)*(RNTI_PER_SUBFRAME)

// Reserverd values
#define ILLEGAL_RNTI 0

typedef enum {
  RMV_FALSE = 0,
  RMV_UNCERTAIN = 1,
  RMV_TRUE = 2,
} RMValidationResult_t;

class RNTIActiveSetItem {
public:
  uint16_t rnti;
  ActivationReason reason;
  RNTIActiveSetItem(uint16_t rnti, ActivationReason reason = RM_ACT_UNSET) :
    rnti(rnti),
    reason(reason) {}
  bool operator==(const RNTIActiveSetItem& other) const { return rnti == other.rnti; }
  bool operator!=(const RNTIActiveSetItem& other) const { return rnti != other.rnti; }
};

class RNTIManager {
public:
  RNTIManager(uint32_t nformats, uint32_t maxCandidatesPerStepPerFormat);
  virtual ~RNTIManager();

  virtual void addEvergreen(uint16_t rntiStart, uint16_t rntiEnd, uint32_t formatIdx);
  virtual void addCandidate(uint16_t rnti, uint32_t formatIdx);
  virtual bool validate(uint16_t rnti, uint32_t formatIdx);
  virtual bool validateAndRefresh(uint16_t rnti, uint32_t formatIdx);
  virtual void activateAndRefresh(uint16_t rnti, uint32_t formatIdx, ActivationReason reason);
  virtual void stepTime();
  virtual void stepTime(uint32_t nSteps);
  virtual uint32_t getFrequency(uint16_t rnti, uint32_t formatIdx);
  virtual uint32_t getAssociatedFormatIdx(uint16_t rnti);
  virtual ActivationReason getActivationReason(uint16_t rnti);
  virtual void getHistogramSummary(uint32_t* buf);
  virtual std::vector<rnti_manager_active_set_t> getActiveSet();
  virtual void printActiveSet();

  static std::string getActivationReasonString(ActivationReason reason);
private:
  RNTIManager(const RNTIManager&);    // prevent copy
  bool validateByEvergreen(uint16_t rnti, uint32_t formatIdx);
  RMValidationResult_t validateByActiveList(uint16_t rnti, uint32_t formatIdx);
  bool validateByHistogram(uint16_t rnti, uint32_t formatIdx);
  virtual uint32_t getLikelyFormatIdx(uint16_t rnti);
  void activateRNTI(uint16_t rnti, ActivationReason reason);
  void deactivateRNTI(uint16_t rnti);
  uint32_t nformats;
  std::vector<Histogram> histograms;
  std::vector<std::vector<Interval> > evergreen;
  std::vector<bool> active;
  std::list<RNTIActiveSetItem> activeSet;
  std::vector<uint32_t> lastSeen;
  std::vector<uint32_t> assocFormatIdx;
  uint32_t timestamp;
  uint32_t lifetime;
  uint32_t threshold;
  uint32_t maxCandidatesPerStepPerFormat;
  std::vector<int32_t> remainingCandidates;
};
