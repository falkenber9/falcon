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

#ifdef __cplusplus
extern "C" {
#endif

#include "srslte/phy/phch/dci.h"

#define CNI_TIMESTAMP

#ifdef CNI_TIMESTAMP
#include <time.h>
#include <sys/time.h>
#endif

// IMDEA OWL's power threshold for blind decoding a set of CCEs
#define PWR_THR				.7f

// DCI minimum average llr for for blind decoding a set of CCEs
#define DCI_MINIMUM_AVG_LLR_BOUND PWR_THR
//#define DCI_MINIMUM_AVG_LLR_BOUND 0.5f     // (for testing FALCON may go downto 0.5)

// RNTI Threshold
#define RNTI_HISTOGRAM_THRESHOLD 5

// RNTI histogram and circular buffer
#define RNTI_HISTOGRAM_ELEMENT_COUNT 65536
#define RNTI_HISTORY_DEPTH_MSEC 200
#define RNTI_PER_SUBFRAME (304/5)
#define RNTI_HISTORY_DEPTH (RNTI_HISTORY_DEPTH_MSEC)*(RNTI_PER_SUBFRAME)

typedef struct {
    uint16_t* rnti_histogram;           // the actual histogram
    uint16_t* rnti_history;             // circular buffer of the recent seen RNTIs
    uint16_t* rnti_history_current;     // pointer to current head/(=foot) of rnti_history
    uint16_t* rnti_history_end;         // pointer to highest index in history array
    int rnti_history_active_users;      // number of currently active RNTIs
    int rnti_histogram_ready;           // ready-indicator, if history is filled
} rnti_histogram_t;

typedef struct {
  int is_active;
  int last_seen;
  srslte_dci_format_t dci_format;
} rnti_active_set_entry_t;

typedef struct {
  rnti_active_set_entry_t entry[65536];
} rnti_active_set_t;

SRSLTE_API void rnti_histogram_init(rnti_histogram_t *h);
SRSLTE_API void rnti_histogram_free(rnti_histogram_t *h);
SRSLTE_API void rnti_histogram_add_rnti(rnti_histogram_t *h, const uint16_t rnti);
SRSLTE_API unsigned int rnti_histogram_get_occurence(rnti_histogram_t *h, const uint16_t rnti);

SRSLTE_API void rnti_active_set_init(rnti_active_set_t *r);
SRSLTE_API void rnti_active_set_free(rnti_active_set_t *r);

typedef struct SRSLTE_API {
  srslte_dci_format_t format;
  uint32_t global_index;
  uint32_t hits;
} falcon_dci_meta_format_t;

typedef struct SRSLTE_API {
  uint32_t L;    // Aggregation level
  uint32_t ncce; // Position of first CCE of the dci
  bool used;     // Flag whether this location contains a valid dci
  bool occupied; // Flag whether this location is overlapped by a valid dci
  bool checked;  // Flag whether this location has already been processed by dci blind search
  bool sufficient_power;    // Flag if this location has enough power to carry any useful information
  float power;   // Average LLR of location (avg of cce.power)
} falcon_dci_location_t;

typedef struct SRSLTE_API {
    falcon_dci_location_t* location[4];   //overlapping location for each aggregation level. CAUTION: might be NULL
    float power;    // Average LLR in CCE
} falcon_cce_to_dci_location_map_t;

SRSLTE_API int falcon_dci_index_of_format_in_list(srslte_dci_format_t format,
                                                  const srslte_dci_format_t* format_list,
                                                  const uint32_t nof_formats);

SRSLTE_API void sprint_hex(char *str, const uint32_t max_str_len, uint8_t *x, const uint32_t len);

#ifdef CNI_TIMESTAMP
// CNI contribution: provide system timestamp to log
SRSLTE_API int srslte_dci_msg_to_trace_timestamp(srslte_dci_msg_t *msg,
                   uint16_t msg_rnti,
                   uint32_t nof_prb,
                   uint32_t nof_ports,
                   srslte_ra_dl_dci_t *dl_dci,
                   srslte_ra_ul_dci_t *ul_dci,
                   srslte_ra_dl_grant_t *dl_grant,
                   srslte_ra_ul_grant_t *ul_grant,
                   uint32_t sf_idx,
                   uint32_t sfn,
                   uint32_t prob,
                   uint32_t ncce,
                   uint32_t aggregation,
                   srslte_dci_format_t format,
                   uint32_t cfi,
                   float power,
                   struct timeval timestamp,
                   uint32_t histval,
                   FILE* dci_file);
#endif

// IMDEA contribution: this function prints the traces for the LTE sniffer either on stdout, file or both
SRSLTE_API int srslte_dci_msg_to_trace(srslte_dci_msg_t *msg,
                   uint16_t msg_rnti,
                   uint32_t nof_prb,
                   uint32_t nof_ports,
                   srslte_ra_dl_dci_t *dl_dci,
                   srslte_ra_ul_dci_t *ul_dci,
                   srslte_ra_dl_grant_t *dl_grant,
                   srslte_ra_ul_grant_t *ul_grant,
                   uint32_t sf_idx,
                   uint32_t sfn,
                   uint32_t prob,
                   uint32_t ncce,
                   uint32_t aggregation,
                   srslte_dci_format_t format,
                   uint32_t cfi,
                   float power,
                   FILE* dci_file);

SRSLTE_API bool falcon_dci_location_isvalid(falcon_dci_location_t *c);

#ifdef __cplusplus
}
#endif
