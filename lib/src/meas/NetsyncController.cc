/*
 * Copyright (c) 2019 Robert Falkenberg.
 *
 * This file is part of FALCON 
 * (see https://github.com/falkenber9/falcon).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */
#include "falcon/meas/NetsyncController.h"

#include <unistd.h>
#include <ctime>

using namespace std;

NetsyncController::NetsyncController() :
  cancelPollThread(false),
  cancelAutoThread(false),
  pollThreadActive(false),
  autoThreadActive(false)
{

}

NetsyncController::~NetsyncController() {
  cancelPollThread = true;
  cancelAutoThread = true;
  if(pollThreadActive) {
    if(!pthread_kill(pollThread, 0)) {
      pthread_cancel(pollThread);
      pthread_join(pollThread, nullptr);
    }
  }
  pollThreadActive = false;

  if(autoThreadActive) {
    if(!pthread_kill(autoThread, 0)) {
      pthread_cancel(autoThread);
      pthread_join(autoThread, nullptr);
    }
  }
  autoThreadActive = false;
}

void NetsyncController::init(shared_ptr<NetsyncMaster> netsyncMaster,
                             uint32_t defaultPollIntervalSec,
                             uint32_t defaultAutoIntervalSec) {
  this->netsyncMaster = netsyncMaster;
  this->defaultPollIntervalSec = defaultPollIntervalSec;
  this->defaultAutoIntervalSec = defaultAutoIntervalSec;
  netsyncMaster->launchReceiver();
}

bool NetsyncController::parse(istream &params) {
  string line, token;
  getline(params, line);

  istringstream linestream(line);
  linestream >> token;

  if(token == "quit" ||
     token == "exit") {
    return false;
  }
  else if(token == "") {
    // nothing here
  }
  else if(token == "help") {
    showHelp();
  }
  else if(token == "start") {
    parseStart(linestream);
  }
  else if(token == "stop") {
    if(netsyncMaster != nullptr) netsyncMaster->stop();
  }
  else if(token == "poll") {
    parsePoll(linestream);
  }
  else if(token == "auto") {
    parseAuto(linestream);
  }
  else if(token == "loc") {
    if(netsyncMaster != nullptr) netsyncMaster->location();
  }
  else {
    unknownToken(token);
  }

  return true;
}

void NetsyncController::showHelp() {
  cout << "Commands:" << endl;
  cout << "\t" << "quit" << "\t" << "Exit application" << endl;
  cout << "\t" << "exit" << "\t" << "Exit application" << endl;
  cout << "\t" << "help" << "\t" << "Show this help message" << endl;
  cout << "\t" << "start DIRECTION [NOF_SUBFRAMES]" << "\t" << "Broadcast start message in DIRECTION=0|1" << endl;
  cout << "\t" << "stop" << "\t" << "Broadcast stop message (stop capturing)" << endl;
  cout << "\t" << "auto start [interval]|stop" << "\t" << "Enable/Disable automatic probing (repeating start 0/1), default " << defaultAutoIntervalSec << endl;
  cout << "\t" << "poll start|stop" << "\t" << "Enable/Disable polling of network status" << endl;
  cout << "\t" << "loc" << "\t" << "Print current location from GPS" << endl;
 }

void NetsyncController::unknownToken(const string token) {
  cout << "Unknown token '" << token << "'" << endl;
}

void NetsyncController::parseStart(istream &params) {
  string token;
  params >> token;
  uint32_t direction = 0;
  if(token == "0") {
    direction = 0;
  }
  else if(token == "1") {
    direction = 1;
  }
  else {
    unknownToken(token);
    return;
  }

  if(netsyncMaster != nullptr) netsyncMaster->start(getTimestampString(), direction);
}

void NetsyncController::parsePoll(istream &params) {
  string token;
  params >> token;
  if(token == "start") {
    if(!pollThreadActive) {
      pollThreadActive = true;
      cancelPollThread = false;
      if(pthread_create(&pollThread, nullptr, pollStart, this)) {
        cerr << "could not create pollThread" << endl;
        pollThreadActive = false;
      }
      else {
        cout << "polling enabled" << endl;
      }
    }
  }
  else if(token == "stop") {
    if(pollThreadActive) {
      cancelPollThread = true;
      if(!pthread_kill(pollThread, 0)) {
        pthread_cancel(pollThread);
        pthread_join(pollThread, nullptr);
      }
      pollThreadActive = false;
      cout << "polling disabled" << endl;
    }
  }
  else {
    unknownToken(token);
  }
}

void* NetsyncController::pollStart(void *obj) {
  static_cast<NetsyncController*>(obj)->pollFunc();
  return nullptr;
}

void NetsyncController::pollFunc() {
  while(!cancelPollThread) {
    if(netsyncMaster != nullptr) netsyncMaster->poll();
    sleep(defaultPollIntervalSec);
  }
}

void NetsyncController::parseAuto(istream &params) {
  string token;
  params >> token;
  if(token == "start") {
    if(!autoThreadActive) {
      if(!params.eof()) {
        params >> token;
        autoIntervalSec = static_cast<uint32_t>(atoi(token.c_str()));
      }
      else {
        autoIntervalSec = defaultAutoIntervalSec;
      }
      autoThreadActive = true;
      cancelAutoThread = false;
      if(pthread_create(&autoThread, nullptr, autoStart, this)) {
        cerr << "could not create autoThread" << endl;
        autoThreadActive = false;
      }
      else {
        cout << "auto mode enabled" << endl;
      }
    }
    else {
      cout << "auto mode already active" << endl;
    }
  }
  else if(token == "stop") {
    if(autoThreadActive) {
      cancelAutoThread = true;
      if(!pthread_kill(autoThread, 0)) {
        pthread_cancel(autoThread);
        pthread_join(autoThread, nullptr);
      }
      autoThreadActive = false;
      cout << "auto mode disabled" << endl;
    }
  }
  else {
    unknownToken(token);
  }
}

void* NetsyncController::autoStart(void *obj) {
  static_cast<NetsyncController*>(obj)->autoFunc();
  return nullptr;
}

void NetsyncController::autoFunc() {
  uint32_t direction = 0;
  while(!cancelAutoThread) {
    if(netsyncMaster != nullptr) netsyncMaster->start(getTimestampString(), direction);
    sleep(autoIntervalSec);
    direction = (direction + 1) % 2;
  }
}

string NetsyncController::getTimestampString() {
  time_t now = time(nullptr);
  tm* now_tm = localtime(&now);
  char c_str[100];
  string nowText;
  size_t ret = strftime(c_str, sizeof(c_str), "%Y-%m-%d-%H-%M-%S", now_tm);
  if(ret > 0) {
    nowText = string(c_str, ret);
  }
  return nowText;
}
