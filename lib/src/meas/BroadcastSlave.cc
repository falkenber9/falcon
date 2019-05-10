#include "falcon/meas/BroadcastSlave.h"

BroadcastSlave::BroadcastSlave(const uint16_t port) :
  bsHandle(nullptr)
{
  bsHandle = broadcast_slave_init(port);
}

BroadcastSlave::~BroadcastSlave() {
  broadcast_slave_destroy(bsHandle);
}

bool BroadcastSlave::replyBytes(const char *buf, size_t length) {
  return broadcast_slave_reply(bsHandle, buf, length) == 0;
}

size_t BroadcastSlave::receiveBytes(char *buf, size_t bufSize) {
  return broadcast_slave_receive(bsHandle, buf, bufSize);
}
