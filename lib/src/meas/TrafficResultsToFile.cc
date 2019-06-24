/*
 * Copyright (c) 2019 Robert Falkenberg.
 *
 * This file is part of FALCON 
 * (see https://github.com/falkenber9/falcon).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */
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
