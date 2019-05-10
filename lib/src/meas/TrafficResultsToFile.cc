#include "falcon/meas/TrafficResultsToFile.h"

#include <iostream>
#include <fstream>

using namespace std;

TrafficResultsToFile::TrafficResultsToFile() :
  trafficGenerator(nullptr),
  outputFileName(),
  delimiter(",")
{

}

TrafficResultsToFile::TrafficResultsToFile(TrafficGenerator *trafficGenerator) :
  trafficGenerator(trafficGenerator),
  outputFileName(),
  delimiter(",")
{

}

TrafficResultsToFile::TrafficResultsToFile(TrafficGenerator *trafficGenerator,
                                           const string &outputFileName) :
  trafficGenerator(trafficGenerator),
  outputFileName(outputFileName),
  delimiter(",")
{

}

void TrafficResultsToFile::setTrafficGenerator(TrafficGenerator *trafficGenerator) {
  this->trafficGenerator = trafficGenerator;
}

void TrafficResultsToFile::setOutputFileName(const string& outputFileName) {
  this->outputFileName = outputFileName;
}

void TrafficResultsToFile::setDelimiter(const string &delimiter) {
  this->delimiter = delimiter;
}

void TrafficResultsToFile::actionBeforeTransfer() {
  // nothing
}

void TrafficResultsToFile::actionDuringTransfer() {
  // nothing
}

void TrafficResultsToFile::actionAfterTransfer() {
  saveResults();
}

void TrafficResultsToFile::saveResults() {
  ofstream outputStream;
  outputStream.open(outputFileName);
  if(outputStream.is_open() && trafficGenerator != nullptr) {
    outputStream << trafficGenerator->getStatus().toCSV(',');
    outputStream.close();
  }
}

TrafficResultsToFileAndNetsyncMessages::TrafficResultsToFileAndNetsyncMessages(TrafficGenerator* trafficGenerator,
                                                                               const string& outputFileName,
                                                                               NetsyncSlave* netsync)
  : TrafficResultsToFile(trafficGenerator, outputFileName),
    netsync(netsync)
{

}

void TrafficResultsToFileAndNetsyncMessages::actionAfterTransfer() {
  if(netsync != nullptr) {
    *netsync << "Transfer finished with state: " << transfer_state_to_string(trafficGenerator->getStatus().state) << endl;
  }
  TrafficResultsToFile::actionAfterTransfer();
}

TrafficResultsToFileAndNetsyncMessagesStopTxPowerSampling::TrafficResultsToFileAndNetsyncMessagesStopTxPowerSampling(TrafficGenerator* trafficGenerator,
                                                                                                                     const string& outputFileName,
                                                                                                                     NetsyncSlave* netsync,
                                                                                                                     AuxModem* modem)
  : TrafficResultsToFileAndNetsyncMessages(trafficGenerator, outputFileName, netsync),
    modem(modem)
{

}

void TrafficResultsToFileAndNetsyncMessagesStopTxPowerSampling::actionAfterTransfer() {
  if(modem != nullptr) {
    modem->stopTXPowerSampling();
  }
  TrafficResultsToFileAndNetsyncMessages::actionAfterTransfer();
}
