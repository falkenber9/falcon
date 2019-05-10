#include "falcon/meas/BroadcastMaster.h"

BroadcastMaster::BroadcastMaster(const string& ip, const uint16_t port) :
  bmHandle(nullptr)
{
  bmHandle = broadcast_master_init(ip.c_str(), port);
}

BroadcastMaster::~BroadcastMaster() {
  broadcast_master_destroy(bmHandle);
}

bool BroadcastMaster::sendBytes(const char *buf, size_t length) {
  return broadcast_master_send(bmHandle, buf, length) == 0;
}

size_t BroadcastMaster::receiveBytes(char *buf, size_t bufSize) {
  return broadcast_master_receive(bmHandle, buf, bufSize);
}
