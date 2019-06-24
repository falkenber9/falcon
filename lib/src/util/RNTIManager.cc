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
#include <iostream>

#include "falcon/util/RNTIManager.h"

using namespace std;

/////////////////////////
/// C wrapper functions
/////////////////////////

void* rnti_manager_create(uint32_t n_formats, uint32_t maxCandidatesPerStepPerFormat) {
  return new RNTIManager(n_formats, maxCandidatesPerStepPerFormat);
}

void rnti_manager_free(void* h) {
  if(h) {
    RNTIManager* manager = static_cast<RNTIManager*>(h);
    delete manager;
  }
}

void rnti_manager_add_evergreen(void* h, uint16_t rnti_start, uint16_t rnti_end, uint32_t format_idx) {
  if(h) static_cast<RNTIManager*>(h)->addEvergreen(rnti_start, rnti_end, format_idx);
}

void rnti_manager_add_candidate(void* h, uint16_t rnti, uint32_t format_idx) {
  if(h) static_cast<RNTIManager*>(h)->addCandidate(rnti, format_idx);
}

int rnti_manager_validate(void* h, uint16_t rnti, uint32_t format_idx) {
  if(h) return static_cast<RNTIManager*>(h)->validate(rnti,format_idx);
  return 0;
}

int rnti_manager_validate_and_refresh(void* h, uint16_t rnti, uint32_t format_idx) {
  if(h) return static_cast<RNTIManager*>(h)->validateAndRefresh(rnti, format_idx);
  return 0;
}

void rnti_manager_activate_and_refresh(void* h, uint16_t rnti, uint32_t format_idx, rnti_manager_activation_reason_t reason) {
  if(h) static_cast<RNTIManager*>(h)->activateAndRefresh(rnti, format_idx, reason);
}

void rnti_manager_step_time(void* h) {
  if(h) static_cast<RNTIManager*>(h)->stepTime();
}

void rnti_manager_step_time_multi(void* h, uint32_t n_steps) {
  if(h) static_cast<RNTIManager*>(h)->stepTime(n_steps);
}

uint32_t rnti_manager_getFrequency(void* h, uint16_t rnti, uint32_t format_idx) {
  if(h) return static_cast<RNTIManager*>(h)->getFrequency(rnti, format_idx);
  return 0;
}

uint32_t rnti_manager_get_associated_format_idx(void* h, uint16_t rnti) {
  if(h) return static_cast<RNTIManager*>(h)->getAssociatedFormatIdx(rnti);
  return 0;
}

rnti_manager_activation_reason_t rnti_manager_get_activation_reason(void* h, uint16_t rnti) {
  if(h) return static_cast<RNTIManager*>(h)->getActivationReason(rnti);
  return RM_ACT_UNSET;
}

void rnti_manager_get_histogram_summary(void* h, uint32_t* buf) {
  if(h) static_cast<RNTIManager*>(h)->getHistogramSummary(buf);
}

uint32_t rnti_manager_get_active_set(void* h, rnti_manager_active_set_t* buf, uint32_t buf_sz) {
  uint32_t i = 0;
  if(h) {
    RNTIManager* rm = static_cast<RNTIManager*>(h);
    std::vector<rnti_manager_active_set_t> activeSet = rm->getActiveSet();
    while(i < activeSet.size() && i < buf_sz) {
      buf[i] = activeSet[i];
      i++;
    }
  }
  return i;
}

void rnti_manager_print_active_set(void* h) {
  if(h) static_cast<RNTIManager*>(h)->printActiveSet();
}

const char* rnti_manager_activation_reason_string(rnti_manager_activation_reason_t reason) {
  return RNTIManager::getActivationReasonString(reason).c_str();
}

////////////////////////
/// C++ class functions
////////////////////////

RNTIManager::RNTIManager(uint32_t nformats, uint32_t maxCandidatesPerStepPerFormat) :
  nformats(nformats),
  histograms(nformats, Histogram(RNTI_HISTORY_DEPTH, RNTI_HISTOGRAM_ELEMENT_COUNT)),
  evergreen(nformats, vector<Interval>()),
  active(RNTI_HISTOGRAM_ELEMENT_COUNT, false),
  activeSet(),
  lastSeen(RNTI_HISTOGRAM_ELEMENT_COUNT, 0),
  assocFormatIdx(RNTI_HISTOGRAM_ELEMENT_COUNT, 0),
  timestamp(0),
  lifetime(RRC_INACTIVITY_TIMER_MS),
  threshold(RNTI_HISTOGRAM_THRESHOLD),
  maxCandidatesPerStepPerFormat(maxCandidatesPerStepPerFormat),
  remainingCandidates(nformats, static_cast<int32_t>(maxCandidatesPerStepPerFormat))
{

}

RNTIManager::~RNTIManager() {

}

void RNTIManager::addEvergreen(uint16_t rntiStart, uint16_t rntiEnd, uint32_t formatIdx) {
  evergreen[formatIdx].push_back(Interval(rntiStart, rntiEnd));
}

void RNTIManager::addCandidate(uint16_t rnti, uint32_t formatIdx) {
  histograms[formatIdx].add(rnti);
  remainingCandidates[formatIdx]--;
}

bool RNTIManager::validate(uint16_t rnti, uint32_t formatIdx) {

  /* evergreen consultation */
  if(validateByEvergreen(rnti, formatIdx)) {
    return true;
  }

  /* active-list consultation */
  RMValidationResult_t rmvRet;
  rmvRet = validateByActiveList(rnti, formatIdx);
  switch (rmvRet) {
    case RMV_TRUE:
      return true;
    case RMV_FALSE:
      return false;
    case RMV_UNCERTAIN:
      /* continue validation */
      break;
  }

  bool ret;
  ret = validateByHistogram(rnti, formatIdx);
  return ret;
}

bool RNTIManager::validateAndRefresh(uint16_t rnti, uint32_t formatIdx) {
  bool result = validate(rnti, formatIdx);
  if(result) {
    lastSeen[rnti] = timestamp;
  }
  return result;
}

void RNTIManager::activateAndRefresh(uint16_t rnti, uint32_t formatIdx, ActivationReason reason) {
  activateRNTI(rnti, reason);
  lastSeen[rnti] = timestamp;
  assocFormatIdx[rnti] = formatIdx;
}

uint32_t RNTIManager::getFrequency(uint16_t rnti, uint32_t formatIdx) {
  return histograms[formatIdx].getFrequency(rnti);
}

uint32_t RNTIManager::getAssociatedFormatIdx(uint16_t rnti) {
  return assocFormatIdx[rnti];
}

ActivationReason RNTIManager::getActivationReason(uint16_t rnti) {
  for(list<RNTIActiveSetItem>::iterator it = activeSet.begin(); it != activeSet.end(); it++) {
    if(it->rnti == rnti) return it->reason;
  }
  return RM_ACT_UNSET;
}

vector<rnti_manager_active_set_t> RNTIManager::getActiveSet() {
  vector<rnti_manager_active_set_t> result(activeSet.size());
  uint32_t index = 0;
  for(list<RNTIActiveSetItem>::iterator it = activeSet.begin(); it != activeSet.end(); it++) {
    result[index].rnti = it->rnti;
    result[index].reason = it->reason;
    result[index].last_seen = timestamp - lastSeen[it->rnti];
    result[index].assoc_format_idx = getAssociatedFormatIdx(it->rnti);
    result[index].frequency = getFrequency(result[index].rnti, result[index].assoc_format_idx);
    if(result[index].assoc_format_idx != 0) {
      result[index].frequency += getFrequency(result[index].rnti, 0);
    }
    index++;
  }
  return result;
}

void RNTIManager::printActiveSet() {
  std::vector<rnti_manager_active_set_t> activeSet = getActiveSet();
  std::vector<rnti_manager_active_set_t>::size_type n_active = activeSet.size();

  printf("--------Active RNTI Set--------\n");
  printf("RNTI\tFormat\tFreq\tLast[ms] Reason\n");
  printf("-------------------------------\n");
  for(uint32_t i = 0; i< n_active; i++) {
    printf("%5d\t%d\t%3d\t%5d\t%s\n",
           activeSet[i].rnti,
           activeSet[i].assoc_format_idx,
           activeSet[i].frequency,
           activeSet[i].last_seen,
           RNTIManager::getActivationReasonString(activeSet[i].reason).c_str());
  }
  printf("-------------------------------\n");
  printf("Total: %ld\n", n_active);
  printf("-------------------------------\n");

}

string RNTIManager::getActivationReasonString(ActivationReason reason) {
  switch(reason) {
    case RM_ACT_UNSET:
      return "unset";
    case RM_ACT_EVERGREEN:
      return "evergreen";
    case RM_ACT_RAR:
      return "random access";
    case RM_ACT_SHORTCUT:
      return "shortcut";
    case RM_ACT_HISTOGRAM:
      return "histogram";
    case RM_ACT_OTHER:
      return "other";
  }
  return "INVALID";
}

void RNTIManager::getHistogramSummary(uint32_t *buf)
{
  memset(buf, 0, RNTI_HISTOGRAM_ELEMENT_COUNT*sizeof(uint32_t));
  for(uint32_t i=0; i<nformats; i++) {   //nformats
    const uint32_t* histData = histograms[i].getFrequencyAll();
    for(uint32_t j=0; j<RNTI_HISTOGRAM_ELEMENT_COUNT; j++) {
      buf[j] += histData[j];
    }
  }
}

bool RNTIManager::validateByEvergreen(uint16_t rnti, uint32_t formatIdx) {
  vector<Interval>& intervals = evergreen[formatIdx];
  for(vector<Interval>::iterator inter = intervals.begin(); inter != intervals.end(); inter++) {
    if(inter->matches(rnti)) return true;
  }
  return false;
}

RMValidationResult_t RNTIManager::validateByActiveList(uint16_t rnti, uint32_t formatIdx) {
  if(active[rnti]) {  /* active RNTI */
    if(timestamp - lastSeen[rnti] < lifetime) {   /* lifetime check */
      if(formatIdx == 0) return RMV_TRUE; /* always accept 0 (uplink) */
      if(assocFormatIdx[rnti] > 0) {  /* downlink format is already fixed */
        if(assocFormatIdx[rnti] == formatIdx) {
          return RMV_TRUE;
        }
        else {
          /* active RNTI, but mismatching format */
          //return RMV_FALSE;
          // maybe format has changed (a TM allows two different DCI formats)
          return RMV_UNCERTAIN;
        }
      }
      /* RNTI is in active list, but dl format not yet known for sure */
    }
    else {  /* lifetime expired, deactivate RNTI */
      deactivateRNTI(rnti);
    }
  }
  return RMV_UNCERTAIN;
}

bool RNTIManager::validateByHistogram(uint16_t rnti, uint32_t formatIdx) {
  uint32_t ul_frequency = histograms[0].getFrequency(rnti);
  uint32_t dl_frequency = 0;
  uint32_t dlFormatIdx = getLikelyFormatIdx(rnti);
  if(dlFormatIdx > 0) {
    dl_frequency = histograms[dlFormatIdx].getFrequency(rnti);
  }

  if(ul_frequency + dl_frequency > threshold) {
    activateRNTI(rnti, RM_ACT_HISTOGRAM);
    /* assocFormat remains uncertain (=0) until dl_frequency exceeds threshold */
    assocFormatIdx[rnti] = dl_frequency > threshold ? dlFormatIdx : 0;
    //cout << "Activated RNTI " << rnti << "(freq ul/dl " << ul_frequency << " " << dl_frequency << " assocFormatIdx " << assocFormatIdx[rnti] << ")" << endl;
    return formatIdx == dlFormatIdx;
  }

  /* too low frequency... reject! */
  return false;
}

uint32_t RNTIManager::getLikelyFormatIdx(uint16_t rnti) {
  uint32_t result = 0;
  uint32_t max_frequency = 0;
  uint32_t cur_frequency = 0;
  // start here from formatIdx 1, since format 0 is UL anyway
  for(uint32_t formatIdx=1; formatIdx<nformats; formatIdx++) {
    cur_frequency = histograms[formatIdx].getFrequency(rnti);
    if(cur_frequency > max_frequency) {
      max_frequency = cur_frequency;
      result = formatIdx;
    }
  }
  return result;
}

void RNTIManager::activateRNTI(uint16_t rnti, ActivationReason reason) {
  if(!active[rnti]) {
    active[rnti] = true;
    activeSet.push_back(RNTIActiveSetItem(rnti, reason));
  }
}

void RNTIManager::deactivateRNTI(uint16_t rnti) {
  if(active[rnti]) {
    active[rnti] = false;
    activeSet.remove(RNTIActiveSetItem(rnti));
  }
}

void RNTIManager::stepTime() {
  /* add padding to histograms */
  for(uint32_t i=0; i<nformats; i++) {
    if(remainingCandidates[i] > 0) {
      histograms[i].add(ILLEGAL_RNTI, static_cast<uint32_t>(remainingCandidates[i]));
    }
    remainingCandidates[i] = static_cast<int32_t>(maxCandidatesPerStepPerFormat); // reset
  }
  timestamp++;
}

void RNTIManager::stepTime(uint32_t nSteps) {
  for(uint32_t i=0; i<nSteps; i++) {
    stepTime();
  }
}
