#pragma once

#include "broadcast_slave.h"
#include <string>

using namespace  std;

class BroadcastSlave {
public:
  BroadcastSlave(const uint16_t port);
  ~BroadcastSlave();
  bool replyBytes(const char* buf, size_t length);
  size_t receiveBytes(char* buf, size_t bufSize);
private:
  broadcast_slave_t* bsHandle;
};
