#include "falcon/meas/TrafficGenerator.h"

#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

TrafficGenerator::TrafficGenerator() :
  hTransfer(nullptr),
  eventHandler(nullptr)
{

}

TrafficGenerator::~TrafficGenerator() {
  release_probe(hTransfer);
  if(eventHandler != nullptr) {
    delete eventHandler;
    eventHandler = nullptr;
  }
}

ProbeResult TrafficGenerator::getStatus() {
  ProbeResult result;
  result.state = transfer_state_t::TS_UNDEFINED;
  result.datarate_dl = 0;
  result.datarate_ul = 0;
  result.total_transfer_time = 0;
  result.payload_size = 0;
  get_probe_status(&result, hTransfer);
  return result;
}

bool TrafficGenerator::isBusy() {
  if (hTransfer == nullptr) return false;

  ProbeResult status(getStatus());
  return status.state > transfer_state_t::TS_UNDEFINED &&
         status.state < transfer_state_t::TS_FINISHED;
}

bool TrafficGenerator::cleanup() {
  if(isBusy()) return false;

  release_probe(hTransfer);
  hTransfer = nullptr;
  return true;
}

bool TrafficGenerator::performUpload(size_t uploadSize, const string &url) {
  bool result = false;
  if(hTransfer == nullptr) {
    hTransfer = uplink_probe(uploadSize, url.c_str(), &ev_funcs_default, eventHandler);
    result = true;
  }
  return result;
}

bool TrafficGenerator::performDownload(size_t maxDownloadSize, const string &url) {
  bool result = false;
  if(hTransfer == nullptr) {
    hTransfer = downlink_probe(maxDownloadSize, url.c_str(), &ev_funcs_default, eventHandler);
    result = true;
  }
  return result;
}


void TrafficGenerator::cancel() {
  cancel_transfer(hTransfer);
}

bool TrafficGenerator::setEventHandler(TrafficGeneratorEventHandler* handler) {
  /* previous eventHandler must be kept alive during active transmission */
  if(!isBusy()) {
    if(eventHandler != nullptr) {
      delete eventHandler;
    }
    eventHandler = handler;
    return true;
  }
  else {
    return false;
  }
}

ostream& operator<<(ostream &os, const ProbeResult& obj) {
  os << transfer_state_to_string(obj.state) << obj.delimiter;
  os << obj.datarate_dl << obj.delimiter;
  os << obj.datarate_ul << obj.delimiter;
  os << obj.total_transfer_time;
  return os;
}

string ProbeResult::toCSV(const char delim) const {
  stringstream stream;
  stream << transfer_state_to_string(state) << delim;
  stream << datarate_dl << delim;
  stream << datarate_ul << delim;
  stream << total_transfer_time << delim;
  stream << payload_size;
  return stream.str();
}

string ProbeResult::fromCSV(const string& str, const char delim) {
  vector<string> tokens;
  string rest = splitString(str, delim, tokens, 5);
  std::vector<string>::iterator token = tokens.begin();

  string tmp;

  stringstream(*token++) >> tmp;
    state = string_to_transfer_state(tmp.c_str());
  stringstream(*token++) >> datarate_dl;
  stringstream(*token++) >> datarate_ul;
  stringstream(*token++) >> total_transfer_time;
  stringstream(*token++) >> payload_size;

  return rest;
}
