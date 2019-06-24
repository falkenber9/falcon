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
#include "falcon/common/SystemInfo.h" 

SystemInfo::SystemInfo() {}

SystemInfo::~SystemInfo() {}

size_t SystemInfo::getFreeRam() {
  size_t mem = 0;
  struct sysinfo sysInfo;
  int ret = sysinfo(&sysInfo);
  if(ret == 0) {
    mem = sysInfo.freeram;
  }
  return mem;
}

size_t SystemInfo::getAvailableRam() {
  size_t mem = 0;
  std::string token;
  std::ifstream infoFile("/proc/meminfo");
  while(infoFile >> token) {
    if(token == "MemAvailable:") {
      if(infoFile >> mem) {
        // file reports in unit kB
        mem *= 1024;
      }
    }
  }
  return mem;
}
