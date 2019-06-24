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
