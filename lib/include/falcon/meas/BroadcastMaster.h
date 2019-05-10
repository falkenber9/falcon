#pragma once

#include "broadcast_master.h"
#include <string>

using namespace std;

class BroadcastMaster {
public:
  BroadcastMaster(const string& ip, const uint16_t port);
  ~BroadcastMaster();
  bool sendBytes(const char* buf, size_t length);
  size_t receiveBytes(char* buf, size_t bufSize);
private:
  broadcast_master_t* bmHandle;
};
