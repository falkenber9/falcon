#pragma once

#include "probe_modem.h"

class TrafficGeneratorEventHandler {
public:
  virtual ~TrafficGeneratorEventHandler() {}

  virtual void actionBeforeTransfer() = 0;
  virtual void actionDuringTransfer() = 0;
  virtual void actionAfterTransfer() = 0;

};

extern "C" void traffic_generator_action_before_transfer(void* obj);
extern "C" void traffic_generator_action_during_transfer(void* obj);
extern "C" void traffic_generator_action_after_transfer(void* obj);

extern const probe_event_handlers_t ev_funcs_default;
