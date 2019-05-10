#include "falcon/meas/NetsyncMaster.h"
#include "falcon/common/Settings.h"
#include <sstream>

using namespace std;

NetsyncMaster::NetsyncMaster(const string& ip, const uint16_t port) :
  NetsyncReceiverBase(),
  broadcastMaster(ip, port),
  nofSubframes(0),
  offsetSubframes(0),
  payloadSize(0),
  urlUL(),
  urlDL(),
  gps(nullptr),
  txPowerSamplingInterval(0)
{

}

NetsyncMaster::~NetsyncMaster() {

}

void NetsyncMaster::init(uint32_t nofSubframes,
                         uint32_t offsetSubframes,
                         size_t payloadSize,
                         const string& urlUL,
                         const string& urlDL,
                         GPS* gps,
                         uint32_t txPowerSamplingInterval)
{
  this->nofSubframes = nofSubframes;
  this->offsetSubframes = offsetSubframes;
  this->payloadSize = payloadSize;
  this->urlUL = urlUL;
  this->urlDL = urlDL;
  this->gps = gps;
  this->txPowerSamplingInterval = txPowerSamplingInterval;
}

void NetsyncMaster::start(const string& id, uint32_t direction) {
  NetsyncMessageStart msg;
  msg.setId(id);
  msg.setNofSubframes(nofSubframes);
  msg.setOffsetSubframes(offsetSubframes);
  msg.setDirection(direction);
  msg.setPayloadSize(payloadSize);
  switch (direction) {
    case DIRECTION_UPLINK:
      msg.setUrl(urlUL);
      break;
    case DIRECTION_DOWNLINK:
      msg.setUrl(urlDL);
      break;
    default:
      cerr << "Invalid transfer direction: " << direction << endl;
      return;
  }
  msg.setLatitude(0.0);
  msg.setLongitude(0.0);
  if(gps != nullptr) {
    GPSFix fix = gps->getFix();
    if(!fix.is_invalid) {
      msg.setLatitude(fix.latitude);
      msg.setLongitude(fix.longitude);
    }
  }
  msg.setTxPowerSamplingInterval(txPowerSamplingInterval);

  stringstream ss(stringstream::out | stringstream::binary);
  ss << msg;
  string str = ss.str();
  cout << "Start: " << str << endl;
  broadcastMaster.sendBytes(str.c_str(), str.length());
}

void NetsyncMaster::stop() {
  NetsyncMessageStop msg;
  stringstream ss(stringstream::out | stringstream::binary);
  ss << msg;
  string str = ss.str();
  broadcastMaster.sendBytes(str.c_str(), str.length());
}

void NetsyncMaster::poll() {
  NetsyncMessagePoll msg;
  stringstream ss(stringstream::out | stringstream::binary);
  ss << msg;
  string str = ss.str();
  broadcastMaster.sendBytes(str.c_str(), str.length());
}

void NetsyncMaster::location() {
  if(gps != nullptr) {
    GPSFix fix = gps->getFix();
    cout << "Location (lat,lon): " << fix.latitude << ", " << fix.longitude << (fix.is_invalid ? " (invalid)" : "") << endl;
  }
  else {
    cout << "Location (lat,lon): unknown" << endl;
  }
}

void NetsyncMaster::handle(NetsyncMessageText msg) {
  cout << "Reply: " << msg.getText() << endl;
}

void NetsyncMaster::receive() {
  while(!cancelThread) {
    size_t size;
    size = broadcastMaster.receiveBytes(recvThreadBuf, recvThreadBufSize);
    parse(recvThreadBuf, size);
  }
}
