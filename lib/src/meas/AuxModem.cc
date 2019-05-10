#include "falcon/meas/AuxModem.h"
#include "falcon/prof/Stopwatch.h"

#include <unistd.h>
#include <vector>

#include <iostream>

using namespace std;



AuxModem::AuxModem() :
  hModem(nullptr),
  operatorNameCache(),
  interval_us(0),
  txPowerSamples(nullptr),
  cancelTxPowerSampleThread(false),
  TxPowerSampleThreadActive(false)
{
  modem_enable_logger(false);
}

AuxModem::~AuxModem() {
  stopTXPowerSampling();
  if(txPowerSamples != nullptr) {
    delete txPowerSamples;
  }
  release_modem(hModem);
}

bool AuxModem::init() {
  if(hModem != nullptr) {
    release_modem(hModem);
    hModem = nullptr;
  }
  hModem = init_modem();
  return hModem != nullptr;
}

bool AuxModem::configure() {
  return configure_modem(hModem);
}

bool AuxModem::isOnline() {
  return is_online_modem(hModem);
}

bool AuxModem::setOnline(bool value) {
  return set_online_modem(hModem, value);
}

bool AuxModem::configureTXPowerSampling(unsigned int interval_us, unsigned long prealloc) {
  if(TxPowerSampleThreadActive) return false;
  this->interval_us = interval_us;
  if(txPowerSamples != nullptr) {
    delete txPowerSamples;
  }
  txPowerSamples = new std::vector<int>();
  txPowerSamples->reserve(sizeof(int) * prealloc);
  return true;
}

void AuxModem::startTXPowerSampling() {
  if(!TxPowerSampleThreadActive) {
    cancelTxPowerSampleThread = false;
    if(pthread_create(&txPowerSampleThread, nullptr, txPowerSamplingEntry, this)) {
      //could not create thread
    }
    else {
      //successfully created thread
      TxPowerSampleThreadActive = true;
    }
  }
}

void AuxModem::stopTXPowerSampling() {
  if(TxPowerSampleThreadActive) {
      cancelTxPowerSampleThread = true;
      pthread_join(txPowerSampleThread, nullptr);
      TxPowerSampleThreadActive = false;
  }
}

std::vector<int> AuxModem::getTXPowerSamples() {
  stopTXPowerSampling();
  return *txPowerSamples;
}

NetworkInfo AuxModem::getNetworkInfo() {
  NetworkInfo result;
  bool valid = modem_get_network_info(hModem, &result);
  result.setValid(valid);
  return result;
}

string sanitizeOperatorName(const string& inputName) {
  string result(inputName);
  size_t pos = 0;
  pos = inputName.find(" ");
  if(pos != string::npos && pos > 0) {
    result = inputName.substr(0, pos);
  }

  return result;
}

string AuxModem::getOperatorName() {
  string result;

  NetworkInfo info(getNetworkInfo());
  if(info.isValid()) {
    result = getOperatorNameLUT(info.lteinfo->mcc * 100 + info.lteinfo->mnc);
  }

  if(result.length() == 0) {
    operator_name_t* name = modem_get_operator_name(hModem);
    if(name != nullptr) {
      result = sanitizeOperatorName(name->name->name_long);
    }
    release_operator_name(name);
    if(result.length() == 0) {
      result = "unknownOperator";
    }
  }
  operatorNameCache = result;
  return result;
}

string AuxModem::getOperatorNameCached() {
  if(operatorNameCache.length() == 0) {
    getOperatorName();
  }
  return operatorNameCache;
}

string AuxModem::getOperatorNameLUT(int mccX100mnc) {
  switch(mccX100mnc) {
    case 26201:
      return "Telekom_";
    case 26202:
      return "Vodafone";
    case 26203:
    case 26207:
      return "O2-DE___";
    default:
      return "";
  }
}


modem_t* AuxModem::getModemHandle() {
  return hModem;
}

bool operator==(const network_info_t& left, const network_info_t& right) {
  return network_info_is_equal(&left, &right) != 0;
}

ostream& operator<<(ostream& os, const network_info_t& obj) {
  os << "RSRP: " << obj.lteinfo->rsrp << "dBm ";
  os << "RSRQ: " << obj.lteinfo->rsrq << "dB ";
  os << "Freq: " << obj.rf_freq << "MHz ";
  os << "PRB: " << obj.nof_prb << " ";
  os << "Cell ID:" << obj.gstatus->cell_id << " ";
  return os;
}

NetworkInfo::NetworkInfo() {
  // temporary object generation of base class
  network_info_t* tmp = alloc_network_info();
  // move tmp into the base object
  *static_cast<network_info*>(this) = *tmp;
  tmp->lteinfo = nullptr;
  tmp->gstatus = nullptr;

  // destroy tmp
  release_network_info(tmp);

  // further initialization of derived object
  valid = false;
}

//NetworkInfo::NetworkInfo(network_info_t&& other) {
//  // move into the base object
//  static_cast<network_info>(*this) = other;
//  other.lteinfo = nullptr;
//  other.gstatus = nullptr;

//  // set valid
//  valid = true;
//}

NetworkInfo::NetworkInfo(NetworkInfo&& other) {
  // move into the base object
  static_cast<network_info>(*this) = other;
  other.lteinfo = nullptr;
  other.gstatus = nullptr;

  // further movement of derived object
  valid = other.valid;
  other.valid = false;
}

NetworkInfo::~NetworkInfo() {
  // temporary object generation of base class
  network_info_t* tmp = alloc_network_info_shallow();
  // move base object into tmp
  *tmp = *this;
  // steal references in base class (avoid dangling pointers)
  lteinfo = nullptr;
  gstatus = nullptr;

  // destroy tmp
  release_network_info(tmp);

  // further descruction of derived object
  // ...nothing yet
}

string NetworkInfo::toCSV(const sw_em7565_gstatus_response_t& obj, const char delim) {
  stringstream stream;
  stream << obj.current_time << delim;
  stream << obj.temperature << delim;
  stream << obj.reset_counter << delim;
  stream << obj.mode << delim;
  stream << obj.system_mode << delim;
  stream << obj.ps_state << delim;
  stream << obj.lte_band << delim;
  stream << obj.lte_bw_MHz << delim;
  stream << obj.lte_rx_chan << delim;
  stream << obj.lte_tx_chan << delim;
  stream << obj.emm_state << delim;
  stream << obj.rrc_state << delim;
  stream << obj.ims_reg_state << delim;
  stream << obj.pcc_rxm_rssi << delim;
  stream << obj.pcc_rxm_rsrp << delim;
  stream << obj.pcc_rxd_rssi << delim;
  stream << obj.pcc_rxd_rsrp << delim;
  stream << obj.tx_power << delim;
  stream << obj.tac << delim;
  stream << obj.rsrq << delim;
  stream << obj.cell_id << delim;
  stream << obj.sinr;
  return stream.str();
}

string NetworkInfo::fromCSV_gstatus(const string& str, const char delim) {
  vector<string> tokens;
  string rest = splitString(str, delim, tokens, 22);
  std::vector<string>::iterator token = tokens.begin();

  // steal/swap gstatus object from new temporary network_info_t
  network_info_t* tmp = alloc_network_info();
  sw_em7565_gstatus_response_t* tmp_gstatus = tmp->gstatus;
  tmp->gstatus = gstatus;
  gstatus = tmp_gstatus;
  release_network_info(tmp);

  sw_em7565_gstatus_response_t& obj = *gstatus;

  stringstream(*token++) >> obj.current_time;
  stringstream(*token++) >> obj.temperature;
  stringstream(*token++) >> obj.reset_counter;
  stringstream(*token++).getline(obj.mode, sizeof (obj.mode));
  stringstream(*token++).getline(obj.system_mode, sizeof (obj.system_mode));
  stringstream(*token++).getline(obj.ps_state, sizeof (obj.ps_state));
  stringstream(*token++) >> obj.lte_band;
  stringstream(*token++) >> obj.lte_bw_MHz;
  stringstream(*token++) >> obj.lte_rx_chan;
  stringstream(*token++) >> obj.lte_tx_chan;
  stringstream(*token++).getline(obj.emm_state, sizeof (obj.emm_state));
  stringstream(*token++).getline(obj.rrc_state, sizeof (obj.rrc_state));
  stringstream(*token++).getline(obj.ims_reg_state, sizeof (obj.ims_reg_state));
  stringstream(*token++) >> obj.pcc_rxm_rssi;
  stringstream(*token++) >> obj.pcc_rxm_rsrp;
  stringstream(*token++) >> obj.pcc_rxd_rssi;
  stringstream(*token++) >> obj.pcc_rxd_rsrp;
  stringstream(*token++) >> obj.tx_power;
  stringstream(*token++) >> obj.tac;
  stringstream(*token++) >> obj.rsrq;
  stringstream(*token++) >> obj.cell_id;
  stringstream(*token++) >> obj.sinr;

  return rest;
}

string NetworkInfo::toCSV(const sw_em7565_lteinfo_response_t& obj, const char delim) {
  stringstream stream;
  stream << obj.earfn << delim;
  stream << obj.mcc << delim;
  stream << obj.mnc << delim;
  stream << obj.tac << delim;
  stream << obj.cid << delim;
  stream << obj.band << delim;
  stream << obj.d << delim;
  stream << obj.u << delim;
  stream << obj.snr << delim;
  stream << obj.pci << delim;
  stream << obj.rsrq << delim;
  stream << obj.rsrp << delim;
  stream << obj.rssi << delim;
  stream << obj.rxlv << delim;
  stream << obj.nof_intrafreq_neighbours << delim;
  stream << obj.nof_interfreq_neighbours;
  return stream.str();
}

string NetworkInfo::fromCSV_lteinfo(const string& str, const char delim) {
  vector<string> tokens;
  string rest = splitString(str, delim, tokens, 16);
  std::vector<string>::iterator token = tokens.begin();

  // steal/swap lteinfo object from new temporary network_info_t
  network_info_t* tmp = alloc_network_info();
  sw_em7565_lteinfo_response_t* tmp_lteinfo = tmp->lteinfo;
  tmp->lteinfo = lteinfo;
  lteinfo = tmp_lteinfo;
  release_network_info(tmp);

  sw_em7565_lteinfo_response_t& obj = *lteinfo;

  stringstream(*token++) >> obj.earfn;
  stringstream(*token++) >> obj.mcc;
  stringstream(*token++) >> obj.mnc;
  stringstream(*token++) >> obj.tac;
  stringstream(*token++) >> obj.cid;
  stringstream(*token++) >> obj.band;
  stringstream(*token++) >> obj.d;
  stringstream(*token++) >> obj.u;
  stringstream(*token++) >> obj.snr;
  stringstream(*token++) >> obj.pci;
  stringstream(*token++) >> obj.rsrq;
  stringstream(*token++) >> obj.rsrp;
  stringstream(*token++) >> obj.rssi;
  stringstream(*token++) >> obj.rxlv;
  stringstream(*token++) >> obj.nof_intrafreq_neighbours;
  stringstream(*token++) >> obj.nof_interfreq_neighbours;

  return rest;
}

string NetworkInfo::toCSV(const char delim) const {
  stringstream stream;
  stream << nof_prb << delim;
  stream << N_id_2 << delim;
  stream << rf_freq << delim;
  if(lteinfo != nullptr) {
    stream << toCSV(*lteinfo, delim) << delim;
  }
  if(gstatus != nullptr) {
    stream << toCSV(*gstatus, delim);
  }
  return stream.str();
}

string NetworkInfo::fromCSV(const string& str, const char delim) {
  vector<string> tokens;
  string rest = splitString(str, delim, tokens, 3);
  std::vector<string>::iterator token = tokens.begin();

  stringstream(*token++) >> nof_prb;
  stringstream(*token++) >> N_id_2;
  stringstream(*token++) >> rf_freq;

  rest = fromCSV_lteinfo(rest, delim);
  rest = fromCSV_gstatus(rest, delim);

  setValid(true);

  return rest;
}

void* AuxModem::txPowerSamplingEntry(void* obj) {
  static_cast<AuxModem*>(obj)->txPowerSampling();
  return nullptr;
}

void AuxModem::txPowerSampling() {
  sw_em7565_gstatus_response_t* gstatus = alloc_gstatus();
  Stopwatch stopwatch;
  timeval sleeptime;
  sleeptime.tv_sec = 0;
  sleeptime.tv_usec = interval_us;

  while(!cancelTxPowerSampleThread) {
    stopwatch.start();
    int ret = modem_get_gstatus(hModem, gstatus);   // this function eats approx 200ms
    if(ret == 1) {
      txPowerSamples->push_back(gstatus->tx_power);
      //std::cout << gstatus->tx_power << std::endl;
    }
    else {
      txPowerSamples->push_back(-1001);
      //std::cout << -1001 << std::endl;
    }
    timeval diff = sleeptime - stopwatch.getAndContinue();
    if(diff.tv_sec >= 0 && diff.tv_usec >= 0) {
      //std::cout << "Sleep: " << diff.tv_sec << ":" << diff.tv_usec << std::endl;
      if(diff.tv_usec > 0) usleep(static_cast<unsigned int>(diff.tv_usec));
      if(diff.tv_sec > 0) sleep(static_cast<unsigned int>(diff.tv_sec));
    }
//    else {
//      std::cout << "Sleep: NONE" << std::endl;
//    }
  }
  release_gstatus(gstatus);
}
