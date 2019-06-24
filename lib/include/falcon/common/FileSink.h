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

