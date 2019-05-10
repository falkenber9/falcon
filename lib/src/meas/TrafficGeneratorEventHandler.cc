#include "falcon/meas/TrafficGeneratorEventHandler.h"

using namespace std;

void traffic_generator_action_before_transfer(void* obj) {
  if(obj) static_cast<TrafficGeneratorEventHandler*>(obj)->actionBeforeTransfer();
}

void traffic_generator_action_during_transfer(void* obj) {
  if(obj) static_cast<TrafficGeneratorEventHandler*>(obj)->actionDuringTransfer();
}

void traffic_generator_action_after_transfer(void* obj) {
  if(obj) static_cast<TrafficGeneratorEventHandler*>(obj)->actionAfterTransfer();
}

const probe_event_handlers_t ev_funcs_default {
  traffic_generator_action_before_transfer,
  traffic_generator_action_during_transfer,
  traffic_generator_action_after_transfer
};
