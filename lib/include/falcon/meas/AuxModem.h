#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include "probe_modem.h"
#include "falcon/common/CSV.h"

#include <pthread.h>
#include <signal.h>

class NetworkInfo : public network_info_t, public CSV {
public:
  NetworkInfo();
  NetworkInfo(network_info_t&& other);  //move constructor takes base class
  NetworkInfo(NetworkInfo&& other);
  NetworkInfo(const NetworkInfo&) = delete; //prevent copy
  NetworkInfo& operator=(const NetworkInfo&) = delete; //prevent copy
  virtual ~NetworkInfo();
  bool isValid() const { return valid; }
  void setValid(bool valid) { this->valid = valid; }
  std::string toCSV(const char delim) const override;
  std::string fromCSV(const std::string& str, const char delim) override;
private:
  static std::string toCSV(const sw_em7565_lteinfo_response_t& obj, const char delim);
  static std::string toCSV(const sw_em7565_gstatus_response_t& obj, const char delim);
  std::string fromCSV_gstatus(const std::string& str, const char delim);
  std::string fromCSV_lteinfo(const std::string& str, const char delim);
  bool valid;
};

class AuxModem {
public:
  AuxModem();
  AuxModem(const AuxModem&) = delete; //prevent copy
  AuxModem& operator=(const AuxModem&) = delete; //prevent copy
  virtual ~AuxModem();
  bool init();
  bool configure();
  bool isOnline();
  bool setOnline(bool value);
  bool configureTXPowerSampling(unsigned int interval_us, unsigned long prealloc=1000);
  void startTXPowerSampling();
  void stopTXPowerSampling();
  std::vector<int> getTXPowerSamples();

  NetworkInfo getNetworkInfo();
  std::string getOperatorName();
  std::string getOperatorNameCached();
  std::string getOperatorNameLUT(int mccX100mnc);
  modem_t* getModemHandle();
private:
  modem_t* hModem;
  std::string operatorNameCache;

  unsigned int interval_us;
  std::vector<int>* txPowerSamples;
  pthread_t txPowerSampleThread;
  volatile bool cancelTxPowerSampleThread;
  volatile bool TxPowerSampleThreadActive;
  static void* txPowerSamplingEntry(void* obj);
  void txPowerSampling();
};

bool operator==(const network_info_t& left, const network_info_t& right);
std::ostream& operator<<(std::ostream& os, const network_info_t& obj);
