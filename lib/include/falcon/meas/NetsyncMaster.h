#pragma once

#include "falcon/meas/NetsyncCommon.h"
#include "falcon/meas/NetsyncReceiverBase.h"
#include "falcon/meas/BroadcastMaster.h"

#include "falcon/meas/GPS.h"


class NetsyncMaster : public NetsyncReceiverBase {
public:
  NetsyncMaster(const string& ip, const uint16_t port);
  ~NetsyncMaster();
  void init(uint32_t nofSubframes,
            uint32_t offsetSubframes,
            size_t payloadSize,
            const string& urlUL,
            const string& urlDL,
            GPS* gps,
            uint32_t txPowerSamplingInterval);
  void start(const string& id, uint32_t direction);
  void stop();
  void poll();
  void location();
protected:
  void handle(NetsyncMessageText msg);
private:
  void receive();
  BroadcastMaster broadcastMaster;
  uint32_t nofSubframes;
  uint32_t offsetSubframes;
  size_t payloadSize;
  string urlUL;
  string urlDL;
  GPS* gps;
  uint32_t txPowerSamplingInterval;
};
