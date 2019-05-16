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
  NetworkInfo(const NetworkInfo& other);  //allow copy
  NetworkInfo& operator=(const NetworkInfo&) = delete; //prevent copy
  virtual ~NetworkInfo() override;
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
  virtual bool init() = 0;
  virtual bool configure() = 0;
  virtual bool isOnline() = 0;
  virtual bool setOnline(bool value) = 0;
  virtual bool configureTXPowerSampling(unsigned int interval_us, unsigned long prealloc=1000) = 0;
  virtual void startTXPowerSampling() = 0;
  virtual void stopTXPowerSampling() = 0;
  virtual std::vector<int> getTXPowerSamples() = 0;

  virtual NetworkInfo getNetworkInfo() = 0;
  virtual std::string getOperatorName() = 0;
  virtual std::string getOperatorNameCached();
  std::string getOperatorNameLUT(int mccX100mnc);

  virtual void inform(uint32_t nof_prb, int pci, double rf_freq);
protected:
  NetworkInfo informedNetworkInfo;
  std::string operatorNameCache;
};

class SierraWirelessAuxModem : public AuxModem {
public:
  SierraWirelessAuxModem();
  SierraWirelessAuxModem(const SierraWirelessAuxModem&) = delete; //prevent copy
  SierraWirelessAuxModem& operator=(const SierraWirelessAuxModem&) = delete; //prevent copy
  virtual ~SierraWirelessAuxModem() override;
  bool init() override;
  bool configure() override;
  bool isOnline() override;
  bool setOnline(bool value) override;
  bool configureTXPowerSampling(unsigned int interval_us, unsigned long prealloc=1000) override;
  void startTXPowerSampling() override;
  void stopTXPowerSampling() override;
  std::vector<int> getTXPowerSamples() override;

  NetworkInfo getNetworkInfo() override;
  std::string getOperatorName() override;
  modem_t* getModemHandle();
private:
  modem_t* hModem;

  unsigned int interval_us;
  std::vector<int>* txPowerSamples;
  pthread_t txPowerSampleThread;
  volatile bool cancelTxPowerSampleThread;
  volatile bool TxPowerSampleThreadActive;
  static void* txPowerSamplingEntry(void* obj);
  void txPowerSampling();
};

class DummyAuxModem : public AuxModem {
public:
  DummyAuxModem();
  DummyAuxModem(const DummyAuxModem&) = delete; //prevent copy
  DummyAuxModem& operator=(const DummyAuxModem&) = delete; //prevent copy
  virtual ~DummyAuxModem() override;
  bool init() override;
  bool configure() override;
  bool isOnline() override;
  bool setOnline(bool value) override;
  bool configureTXPowerSampling(unsigned int interval_us, unsigned long prealloc=1000) override;
  void startTXPowerSampling() override;
  void stopTXPowerSampling() override;
  std::vector<int> getTXPowerSamples() override;
  NetworkInfo getNetworkInfo() override;
  std::string getOperatorName() override;
private:
 bool online;
 std::vector<int>* txPowerSamples;
};

bool operator==(const network_info_t& left, const network_info_t& right);
std::ostream& operator<<(std::ostream& os, const network_info_t& obj);
