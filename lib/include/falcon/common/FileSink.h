#pragma once

#include <string>
#include <cstring>
#include <cstdint>

// include C-only headers
#ifdef __cplusplus
    extern "C" {
#endif

#include "srslte/phy/io/format.h"
#include "srslte/phy/io/filesink.h"


#ifdef __cplusplus
}
#undef I // Fix complex.h #define I nastiness when using C++
#endif

template <typename SampleType>
class FileSink {
public:
  FileSink();
  FileSink(const FileSink&) = delete; //prevent copy
  FileSink& operator=(const FileSink&) = delete; //prevent copy
  virtual ~FileSink() {
    FileSink<SampleType>::close();
  }

  virtual void open(const std::string& filename) {
    char tmp[1024];  /* WTF! srslte_filesink_init takes char*, not const char* ! */
    strncpy(tmp, filename.c_str(), 1024);
    srslte_filesink_init(&filesink, tmp, type);
    isOpen = true;
  }

  virtual void close() {
    if(isOpen) {
      srslte_filesink_free(&filesink);
      isOpen = false;
    }
  }
  virtual size_t write(SampleType* buffer, size_t nSamples) {
    return srslte_filesink_write(&filesink, static_cast<void*>(buffer), nSamples);
  }

private:
  bool isOpen;
  srslte_filesink_t filesink;
  srslte_datatype_t type;
};

