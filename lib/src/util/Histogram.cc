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
#include "falcon/util/Histogram.h"

#include <cstdlib>
#include <string.h>
#include <strings.h>

Histogram::Histogram(uint32_t _itemCount, uint32_t _valueRange) :
  //rnti_history_active_users(0),
  rnti_histogram(_valueRange, 0),
  rnti_history(_itemCount, 0),
  rnti_history_current(0),
  rnti_history_end(_itemCount),
  rnti_histogram_ready(false),
  itemCount(_itemCount),
  valueRange(_valueRange)
{
//  initBuffers();
}

//Histogram::Histogram(const Histogram& other) :
//  rnti_histogram_ready(other.rnti_histogram_ready),
//  itemCount(other.itemCount),
//  valueRange(other.valueRange)
//{
//  initBuffers();
//  memcpy(rnti_histogram, other.rnti_histogram, valueRange);
//  memcpy(rnti_history, other.rnti_history, itemCount);
//  rnti_history_current = rnti_history + (other.rnti_history_current - other.rnti_history);
//}

//void Histogram::initBuffers() {
//  // init histogram
//  rnti_histogram = new uint32_t[valueRange]();

//  // init history circular buffer
//  rnti_history = new uint16_t[itemCount]();
//  rnti_history_current = rnti_history;
//  rnti_history_end = rnti_history+(itemCount)-1;
//}

Histogram::~Histogram() {
//  delete [] rnti_histogram;
//  delete [] rnti_history;
}

void Histogram::add(uint16_t item) {
  add(item, 1);
}

void Histogram::add(uint16_t item, uint32_t nTimes) {
  while(nTimes-- > 0) {
    if(rnti_histogram_ready) {
      //    if(rnti_histogram[*h->rnti_history_current] == RNTI_HISTOGRAM_THRESHOLD) {  // this rnti is no longer in use, decrement active user counter
      //      rnti_history_active_users--;
      //    }
      rnti_histogram[rnti_history[rnti_history_current]]--;    // decrement occurence counter for old rnti
      //printf("Decremented RNTI %d to %d\n", *h->rnti_history_current, h->rnti_histogram[*h->rnti_history_current]);
    }
    rnti_history[rnti_history_current] = item;               // add new rnti to history
    rnti_histogram[item]++;                     // increment occurence counter for new rnti
    //  if(rnti_histogram[item] == RNTI_HISTOGRAM_THRESHOLD) {  // this rnti reaches threshold -> increment active user counter
    //    rnti_history_active_users++;
    //  }

    rnti_history_current++;
    if(rnti_history_current == rnti_history_end) {  // set current to next element in history
      rnti_histogram_ready = true;                   // first wrap around: histogram is ready now
      rnti_history_current = 0;
    }
  }
}

uint32_t Histogram::getFrequency(uint16_t item) const {
  return rnti_histogram[item];
}

const uint32_t* Histogram::getFrequencyAll() const
{
  return rnti_histogram.data();
}

bool Histogram::ready() const {
  return rnti_histogram_ready;
}

uint32_t Histogram::getItemCount() const {
  return itemCount;
}

uint32_t Histogram::getValueRange() const {
  return valueRange;
}
