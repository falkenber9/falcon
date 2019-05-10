#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <pthread.h>
#include <signal.h>

#include "falcon/meas/NetsyncMaster.h"

class NetsyncController {
public:
  NetsyncController();
  ~NetsyncController();
  void init(std::shared_ptr<NetsyncMaster> netsyncMaster,
            uint32_t defaultPollIntervalSec,
            uint32_t defaultAutoIntervalSec);
  bool parse(std::istream& params);
private:
  void showHelp();
  void unknownToken(const std::string token);
  void parsePoll(std::istream& params);
  void parseAuto(std::istream &params);
  void parseStart(std::istream& params);
  static void* pollStart(void* obj);
  static void* autoStart(void* obj);
  void pollFunc();
  void autoFunc();
  string getTimestampString();

  std::shared_ptr<NetsyncMaster> netsyncMaster;
  uint32_t defaultPollIntervalSec;
  uint32_t defaultAutoIntervalSec;
  uint32_t autoIntervalSec;
  pthread_t pollThread;
  pthread_t autoThread;
  volatile bool cancelPollThread;
  volatile bool cancelAutoThread;
  volatile bool pollThreadActive;
  volatile bool autoThreadActive;
};
