#pragma once

#include "FileSink.h"
#include <cstring>

template <typename SampleType>
class BufferedFileSink : public FileSink<SampleType> {
private:
  SampleType* mem;
  size_t memSize;
  size_t nSamplesWritten;
  std::string filename;
public:
  BufferedFileSink() :
    FileSink<SampleType>(),
    mem(nullptr),
    memSize(0),
    nSamplesWritten(0)
  {}
  BufferedFileSink(const BufferedFileSink&) = delete; //prevent copy
  BufferedFileSink& operator=(const BufferedFileSink&) = delete; //prevent copy
  virtual ~BufferedFileSink() {
    if(mem != nullptr) {
      delete[] mem;
      mem = nullptr;
      memSize = 0;
    }
    nSamplesWritten = 0;
  }

  virtual void open(const std::string& filename) override {
    this->filename = filename;
  }

  virtual void close() override {
    if(mem != nullptr) {
      FileSink<SampleType>::open(filename);
      FileSink<SampleType>::write(mem, nSamplesWritten);
      FileSink<SampleType>::close();
    }

    nSamplesWritten = 0;
  }

  virtual size_t write(SampleType* buffer, size_t nSamples) override {
    // overflow protection
    size_t remainingSampleSpace = memSize - (nSamplesWritten * sizeof(SampleType));
    if(remainingSampleSpace < nSamples) {
      nSamples = remainingSampleSpace;
    }

    // copy + increment
    memcpy(&mem[nSamplesWritten], buffer, nSamples * sizeof(SampleType));
    nSamplesWritten+=nSamples;

    return nSamples;
  }

  virtual void allocate(size_t size) {
    if(mem != nullptr) {
      delete[] mem;
      mem = nullptr;
    }
    memSize = 0;
    nSamplesWritten = 0;

    mem = new SampleType[size/sizeof(SampleType)];
    memSize = size;
    nSamplesWritten = 0;
  }

};
