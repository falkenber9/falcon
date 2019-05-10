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
