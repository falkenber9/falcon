#pragma once

#include <memory>
#include <string>
#include "probe_modem.h"
#include "falcon/common/CSV.h"

#include "TrafficGeneratorEventHandler.h"

class ProbeResult : public probe_result_t, public CSV {
  public:
  ProbeResult() : delimiter(",") {}  
  void setDelimiter(const std::string dlm) { delimiter = dlm; }
  std::string toCSV(const char delim) const override;
  std::string fromCSV(const std::string& str, const char delim) override;
private:
  std::string delimiter;
  friend std::ostream& operator<<(std::ostream &os, const ProbeResult& obj);
};

class TrafficGenerator {
public:
  TrafficGenerator();
  TrafficGenerator(const TrafficGenerator&) = delete; //prevent copy
  TrafficGenerator& operator=(const TrafficGenerator&) = delete; //prevent copy
  virtual ~TrafficGenerator();
  ProbeResult getStatus();
  bool isBusy();
  bool cleanup();
  bool performUpload(size_t uploadSize, const std::string& url);
  bool performDownload(size_t maxDownloadSize, const std::string& url);
  void cancel();

  bool setEventHandler(TrafficGeneratorEventHandler* handler);
private:
  datatransfer_thread_t* hTransfer;
  TrafficGeneratorEventHandler* eventHandler;
};
