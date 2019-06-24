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
