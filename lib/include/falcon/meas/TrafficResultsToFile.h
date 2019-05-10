#pragma once

#include "TrafficGeneratorEventHandler.h"
#include "falcon/meas/TrafficGenerator.h"
#include "falcon/meas/NetsyncSlave.h"
#include <string>
#include <iostream>
#include <ostream>

class TrafficResultsToFile : public TrafficGeneratorEventHandler {
public:
  TrafficResultsToFile();
  TrafficResultsToFile(TrafficGenerator* trafficGenerator);
  TrafficResultsToFile(TrafficGenerator* trafficGenerator,
                       const std::string &outputFileName);
  virtual ~TrafficResultsToFile() override {}

  void setTrafficGenerator(TrafficGenerator* trafficGenerator);
  void setOutputFileName(const std::string &outputFileName);
  void setDelimiter(const std::string &delimiter);

  /* base class interface */
  void actionBeforeTransfer() override;
  void actionDuringTransfer() override;
  void actionAfterTransfer() override;

protected:
  TrafficGenerator* trafficGenerator;
private:
  std::string outputFileName;
  std::string delimiter;

  void saveResults();
};

class TrafficResultsToFileAndNetsyncMessages : public TrafficResultsToFile {
public:
  TrafficResultsToFileAndNetsyncMessages(TrafficGenerator* trafficGenerator,
                                         const std::string &outputFileName,
                                         NetsyncSlave* netsync);

  /* base class interface */
  void actionAfterTransfer() override;
private:
  NetsyncSlave* netsync;
};

class TrafficResultsToFileAndNetsyncMessagesStopTxPowerSampling : public TrafficResultsToFileAndNetsyncMessages {
public:
  TrafficResultsToFileAndNetsyncMessagesStopTxPowerSampling(TrafficGenerator* trafficGenerator,
                                         const std::string &outputFileName,
                                         NetsyncSlave* netsync,
                                         AuxModem* modem);

  /* base class interface */
  void actionAfterTransfer() override;
private:
  AuxModem* modem;
};
