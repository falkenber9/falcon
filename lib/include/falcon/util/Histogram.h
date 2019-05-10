#pragma once

#include <stdint.h>
#include <vector>

class Histogram {
public:
  Histogram(uint32_t itemCount, uint32_t valueRange);
  //Histogram(const Histogram& other);
  virtual ~Histogram();
  virtual void add(uint16_t item);
  virtual void add(uint16_t item, uint32_t nTimes);
  virtual uint32_t getFrequency(uint16_t item) const;
  virtual const uint32_t* getFrequencyAll() const;
  virtual bool ready() const;
  virtual uint32_t getItemCount() const;
  virtual uint32_t getValueRange() const;
private:
  //void initBuffers();
  std::vector<uint32_t> rnti_histogram;           // the actual histogram
  std::vector<uint16_t> rnti_history;             // circular buffer of the recent seen RNTIs
  uint32_t rnti_history_current;     // index to current head/(=foot) of rnti_history
  uint32_t rnti_history_end;         // index to highest index in history array
  //int rnti_history_active_users;      // number of currently active RNTIs
  bool rnti_histogram_ready;           // ready-indicator, if history is filled
  uint32_t itemCount;
  uint32_t valueRange;
};
