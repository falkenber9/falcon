#pragma once

#include "TrafficGeneratorEventHandler.h"

class DummyEventHandler : public TrafficGeneratorEventHandler {
public:
  virtual ~DummyEventHandler() {}

  void actionBeforeTransfer();
  void actionDuringTransfer();
  void actionAfterTransfer();
};
