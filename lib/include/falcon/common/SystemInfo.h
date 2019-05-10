#pragma once

#include <cstddef>
#include <sys/sysinfo.h>
#include <fstream>

#include <iostream>

class SystemInfo {
private:  // no instances at all
  SystemInfo();
  SystemInfo(const SystemInfo&) = delete; //prevent copy
  SystemInfo& operator=(const SystemInfo&) = delete; //prevent copy
public:
  virtual ~SystemInfo();
  static size_t getFreeRam();
  static size_t getAvailableRam();
};
