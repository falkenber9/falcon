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

//#include "src/gui/plot_data.h" //MoHACKS

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>


#include "falcon/phy/falcon_phch/falcon_dci.h"
#include "falcon/phy/common/falcon_phy_common.h"

#include "srslte/phy/utils/bit.h"
#include "srslte/phy/utils/debug.h"
//#define ENABLE_DCI_LOGGING

void sprint_hex(char *str, const uint32_t max_str_len, uint8_t *x, const uint32_t len) {
  uint32_t i, nbytes;
  uint8_t byte;
  nbytes = len/8;
  // check that hex string fits in buffer (every byte takes 2 characters)
  if ((2*(len/8 + ((len%8)?1:0))) >= max_str_len) {
    ERROR("Buffer too small for printing hex string (max_str_len=%d, payload_len=%d).\n", max_str_len, len);
    return;
  }

  int n=0;
  for (i=0;i<nbytes;i++) {
    byte = (uint8_t) srslte_bit_pack(&x, 8);
    n+=sprintf(&str[n], "%02x", byte);
  }
  if (len%8) {
    byte = (uint8_t) srslte_bit_pack(&x, len%8)<<(8-(len%8));
    n+=sprintf(&str[n], "%02x", byte);
  }
  str[n] = 0;
  str[max_str_len-1] = 0;
}

void sscan_hex(const char *str, uint8_t *x, const uint32_t len) {
  size_t str_len = strlen(str);
  uint8_t byte;

  if( (2*(len/8 + (len%8?1:0))) > str_len ) {
    ERROR("Hex string too short (%ld byte) to fill bit string of length %d", str_len, len);
    return;
  }

  int pos=0;
  for(uint32_t n=0; n<len; n+=8) {
    sscanf(&str[pos], "%02hhx", &byte); // read two chars (as hex)
    pos += 2;
    srslte_bit_unpack(byte, &x, 8);
  }
  if(len%8) {
    sscanf(&str[pos], "%02hhx", &byte);
    srslte_bit_unpack(byte<<(8-(len%8)), &x, len%8);
  }

}

SRSLTE_API void rnti_histogram_init(rnti_histogram_t *h) {

  // init histogram
  h->rnti_histogram = calloc(RNTI_HISTOGRAM_ELEMENT_COUNT, sizeof(uint16_t));
  //memset(h->rnti_histogram, 0, sizeof(h->rnti_histogram));

  // init history circular buffer
  h->rnti_history = calloc(RNTI_HISTORY_DEPTH, sizeof(uint16_t));
  h->rnti_history_current = h->rnti_history;
  h->rnti_history_end = h->rnti_history+(RNTI_HISTORY_DEPTH)-1;
  //memset(h->rnti_history, 0, sizeof(h->rnti_history));

  h->rnti_history_active_users = 0;
  h->rnti_histogram_ready = 0;
}

SRSLTE_API void rnti_histogram_free(rnti_histogram_t *h) {
  if(h) {
    free(h->rnti_histogram);
    free(h->rnti_history);
    h->rnti_histogram = 0;
    h->rnti_history = 0;
    h->rnti_history_active_users = 0;
    h->rnti_histogram_ready = 0;
  }
}

SRSLTE_API void rnti_histogram_add_rnti(rnti_histogram_t *h, const uint16_t new_rnti) {
  if(h->rnti_histogram_ready) {
    if(h->rnti_histogram[*h->rnti_history_current] == RNTI_HISTOGRAM_THRESHOLD) {  // this rnti is no longer in use, decrement active user counter
      h->rnti_history_active_users--;
    }
    h->rnti_histogram[*h->rnti_history_current]--;    // decrement occurence counter for old rnti
    //printf("Decremented RNTI %d to %d\n", *h->rnti_history_current, h->rnti_histogram[*h->rnti_history_current]);
  }
  *h->rnti_history_current = new_rnti;               // add new rnti to history
  h->rnti_histogram[new_rnti]++;                     // increment occurence counter for new rnti
  if(h->rnti_histogram[new_rnti] == RNTI_HISTOGRAM_THRESHOLD) {  // this rnti reaches threshold -> increment active user counter
    h->rnti_history_active_users++;
  }

  if(h->rnti_history_current == h->rnti_history_end) {  // set current to next element in history
    h->rnti_histogram_ready = 1;                   // first wrap around: histogram is ready now
    h->rnti_history_current = h->rnti_history;
  }
  else {
    h->rnti_history_current++;
  }
}

SRSLTE_API unsigned int rnti_histogram_get_occurence(rnti_histogram_t *h, const uint16_t rnti) {
  return h->rnti_histogram[rnti];
}

int falcon_dci_index_of_format_in_list(srslte_dci_format_t format,
                                       const srslte_dci_format_t* format_list,
                                       const uint32_t nof_formats) {
  for(uint32_t idx=0; idx<nof_formats; idx++) {
    if(format == format_list[idx]) return (int)idx;
  }
  return -1;
}

#ifdef CNI_TIMESTAMP
// CNI contribution: provide system timestamp to log
int srslte_dci_msg_to_trace_timestamp(srslte_dci_msg_t *msg,
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
                                      FILE* dci_file)
{
  int ret = SRSLTE_ERROR_INVALID_INPUTS;
  bool crc_is_crnti;

  if (msg               !=  NULL) {

    ret = SRSLTE_ERROR;

    char hex_str[SRSLTE_DCI_MAX_BITS/4 + 1];
    sprint_hex(hex_str, sizeof(hex_str), msg->data, msg->nof_bits);

#if 0 // Verify pack/unpack hex strings
    uint8_t dci_bits[SRSLTE_DCI_MAX_BITS];
    sscan_hex(hex_str, dci_bits, msg->nof_bits);
    for(int kk = 0; kk<msg->nof_bits; kk++) {
      if(dci_bits[kk] != msg->data[kk]) {
        ERROR("Mismatch in re-unpacked hex strings");
      }
    }
#endif

    bzero(ul_dci, sizeof(srslte_ra_ul_dci_t));
    bzero(ul_grant, sizeof(srslte_ra_ul_grant_t));
    bzero(dl_dci, sizeof(srslte_ra_dl_dci_t));
    bzero(dl_grant, sizeof(srslte_ra_dl_grant_t));
    crc_is_crnti = false;

    if (msg_rnti >= SRSLTE_CRNTI_START && msg_rnti <= SRSLTE_CRNTI_END) {
      crc_is_crnti = true;
    }

    if (format==SRSLTE_DCI_FORMAT0) {
      if (msg->data[0]==0) {
        if (srslte_dci_msg_unpack_pusch(msg, ul_dci, nof_prb)) {
          DEBUG("Can't unpack UL DCI message: most likely SPS\n");
          return ret;
        }

        if (srslte_ra_ul_dci_to_grant(ul_dci, nof_prb, 0, ul_grant)) {
          INFO("Invalid uplink resource allocation\n");
          return ret;
        }
        if (crc_is_crnti == true) {
          if (ul_dci->mcs_idx < 29) {
#ifdef ENABLE_DCI_LOGGING
            fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t0\t"
                            "%d\t%d\t%d\t%d\t%d\t"
                            "0\t%d\t-1\t%d\t"
                            "%d\t%d\t%d\t%d\t%d\t%s\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                    ul_grant->mcs.idx, ul_grant->L_prb, ul_grant->mcs.tbs, -1, -1,
                    ul_dci->ndi, (10*sfn+sf_idx)%8,
                    ncce, aggregation, cfi, histval, msg->nof_bits, hex_str);
#endif
          }
          else {
#ifdef ENABLE_DCI_LOGGING
            fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t0\t"
                            "%d\t%d\t%d\t%d\t%d\t"
                            "0\t%d\t-1\t%d\t"
                            "%d\t%d\t%d\t%d\t%d\t%s\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                    ul_grant->mcs.idx, ul_grant->L_prb, 0, -1, -1,
                    ul_dci->ndi, (10*sfn+sf_idx)%8,
                    ncce, aggregation, cfi, histval, msg->nof_bits, hex_str);
#endif
          }
        }
        else {
          INFO("Invalid RNTI for uplink\n");
          return ret;
        }
        ret = SRSLTE_SUCCESS;
      }
      else {
        ERROR("wrong upload\n");
      }
    }
    else {
      msg->format = format;
      if (srslte_dci_msg_unpack_pdsch(msg, dl_dci, nof_prb, nof_ports, crc_is_crnti)) { // maybe move inside previous if
        DEBUG("Can't unpack DL DCI message: most likely SPS\n");
        return ret;
      }

      // CNI Fix:
      // dci_to_grant function returns SRSLTE_ERROR in case of HARQ retransmission.
      // Prior versions of SRSLTE have saved and restored TBS for the associated
      // HARQ process instead with the help of global variables.
      // To maintain OWLs behaviour, we mimic this behaviour here
      if (srslte_ra_dl_dci_to_grant(dl_dci, nof_prb, msg_rnti, dl_grant)!=SRSLTE_SUCCESS) {
        INFO("srslte_ra_dl_dci_to_grant did not return SRSLTE_SUCCESS. Probably HARQ or illegal RB alloc\n");
        //do not return error here. Instead continue due to SRSLTE BUG #318
      }

      if(crc_is_crnti == true) {  // HARQ only for C-RNTI
        // We know, last_dl_tbs needs to be saved on a per-RNTI basis.
        // However, we maintain OWLs initial behaviour here
//TODO: Fix rance condition upon multithreaded processing
//FIXME: Any instance of DCICollection writes/reads this variable !!
        static int32_t last_dl_tbs[8][SRSLTE_MAX_CODEWORDS] = {{0}};

        // Set last TBS for this TB (pid) in case of mcs>28 (7.1.7.2 of 36.213)
        for (int i=0;i<SRSLTE_MAX_CODEWORDS;i++) {
          if (dl_grant->mcs[i].idx > 28) {
            dl_grant->mcs[i].tbs = last_dl_tbs[dl_dci->harq_process][i];
          }
          if(dl_grant->mcs[i].tbs < 0) {
            ERROR("Invalid TBS size for PDSCH grant\n");
            dl_grant->mcs[i].tbs = 0;
          }
          // save it for next time
          last_dl_tbs[dl_dci->harq_process][i] = dl_grant->mcs[i].tbs;
        }
      }
      // End CNI Fix

      switch(msg->format) {
        case SRSLTE_DCI_FORMAT0:
          ERROR("Error: no reason to be here\n");
          break;
        case SRSLTE_DCI_FORMAT1:
        case SRSLTE_DCI_FORMAT1A:
        case SRSLTE_DCI_FORMAT1C:
        case SRSLTE_DCI_FORMAT1B:
        case SRSLTE_DCI_FORMAT1D:
#ifdef ENABLE_DCI_LOGGING
          fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t1\t"
                          "%d\t%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t%d\t%s\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                  dl_grant->mcs[0].idx, dl_grant->nof_prb, dl_grant->mcs[0].tbs, -1, -1,
              msg->format+1, dl_dci->ndi, -1, dl_dci->harq_process,
              ncce, aggregation, cfi, histval, msg->nof_bits, hex_str);
#endif
          break;
        case SRSLTE_DCI_FORMAT2:
        case SRSLTE_DCI_FORMAT2A:
        case SRSLTE_DCI_FORMAT2B:
#ifdef ENABLE_DCI_LOGGING
          fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t1\t"
                          "%d\t%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t%d\t%s\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                  dl_grant->mcs[0].idx, dl_grant->nof_prb, dl_grant->mcs[0].tbs + dl_grant->mcs[1].tbs, dl_grant->mcs[0].tbs, dl_grant->mcs[1].tbs,
              msg->format+1, dl_dci->ndi, dl_dci->ndi_1, dl_dci->harq_process,
              ncce, aggregation, cfi, histval, msg->nof_bits, hex_str);
#endif
          break;
          //case SRSLTE_DCI_FORMAT3:
          //case SRSLTE_DCI_FORMAT3A:
        default:
          ERROR("Other formats\n");
      }
      ret = SRSLTE_SUCCESS;
    }
  }
  return ret;
}
#endif

// IMDEA contribution: this function prints the traces for the LTE sniffer either on stdout, file or both
int srslte_dci_msg_to_trace(srslte_dci_msg_t *msg,
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
                            FILE* dci_file)
{
  int ret = SRSLTE_ERROR_INVALID_INPUTS;
  bool crc_is_crnti;

  if (msg               !=  NULL) {

    ret = SRSLTE_ERROR;

    bzero(ul_dci, sizeof(srslte_ra_ul_dci_t));
    bzero(ul_grant, sizeof(srslte_ra_ul_grant_t));
    bzero(dl_dci, sizeof(srslte_ra_dl_dci_t));
    bzero(dl_grant, sizeof(srslte_ra_dl_grant_t));
    crc_is_crnti = false;

    if (msg_rnti >= SRSLTE_CRNTI_START && msg_rnti <= SRSLTE_CRNTI_END) {
      crc_is_crnti = true;
    }

    if (format==SRSLTE_DCI_FORMAT0) {
      if (msg->data[0]==0) {
        if (srslte_dci_msg_unpack_pusch(msg, ul_dci, nof_prb)) {
          DEBUG("Can't unpack UL DCI message: most likely SPS\n");
          return ret;
        }

        if (srslte_ra_ul_dci_to_grant(ul_dci, nof_prb, 0, ul_grant)) {
          INFO("Invalid uplink resource allocation\n");
          return ret;
        }
        if (crc_is_crnti == true) {
          if (ul_dci->mcs_idx < 29) {
            fprintf(dci_file, "%04d\t%d\t%d\t0\t"
                            "%d\t%d\t%d\t%d\t%d\t"
                            "0\t%d\t-1\t%d\t"
                            "%d\t%d\t%d\t%d\n", sfn, sf_idx, msg_rnti,
                    ul_grant->mcs.idx, ul_grant->L_prb, ul_grant->mcs.tbs, -1, -1,
                    ul_dci->ndi, (10*sfn+sf_idx)%8,
                    ncce, aggregation, cfi, prob);
          }
          else {
            fprintf(dci_file, "%04d\t%d\t%d\t0\t"
                            "%d\t%d\t%d\t%d\t%d\t"
                            "0\t%d\t-1\t%d\t"
                            "%d\t%d\t%d\t%d\n", sfn, sf_idx, msg_rnti,
                    ul_grant->mcs.idx, ul_grant->L_prb, 0, -1, -1,
                    ul_dci->ndi, (10*sfn+sf_idx)%8,
                    ncce, aggregation, cfi, prob);
          }
        }
        else {
          INFO("Invalid RNTI for uplink\n");
          return ret;
        }
        ret = SRSLTE_SUCCESS;
      }
      else {
        ERROR("wrong upload\n");
      }
    }
    else {
      msg->format = format;
      if (srslte_dci_msg_unpack_pdsch(msg, dl_dci, nof_prb, nof_ports, crc_is_crnti)) { // maybe move inside previous if
        DEBUG("Can't unpack DL DCI message: most likely SPS\n");
        return ret;
      }

      // CNI Fix:
      // dci_to_grant function returns SRSLTE_ERROR in case of HARQ retransmission.
      // Prior versions of SRSLTE have saved and restored TBS for the associated
      // HARQ process instead with the help of global variables.
      // To maintain OWLs behaviour, we mimic this behaviour here
      if (srslte_ra_dl_dci_to_grant(dl_dci, nof_prb, msg_rnti, dl_grant)!=SRSLTE_SUCCESS) {
        INFO("srslte_ra_dl_dci_to_grant did not return SRSLTE_SUCCESS. Probably HARQ or illegal RB alloc\n");
        //do not return error here. Instead continue due to SRSLTE BUG #318
      }

      if(crc_is_crnti == true) {  // HARQ only for C-RNTI
        // We know, last_dl_tbs needs to be saved on a per-RNTI basis.
        // However, we maintain OWLs initial behaviour here
        static int32_t last_dl_tbs[8][SRSLTE_MAX_CODEWORDS] = {{0}};

        // Set last TBS for this TB (pid) in case of mcs>28 (7.1.7.2 of 36.213)
        for (int i=0;i<SRSLTE_MAX_CODEWORDS;i++) {
          if (dl_grant->mcs[i].idx > 28) {
            dl_grant->mcs[i].tbs = last_dl_tbs[dl_dci->harq_process][i];
          }
          if(dl_grant->mcs[i].tbs < 0) {
            ERROR("Invalid TBS size for PDSCH grant\n");
            dl_grant->mcs[i].tbs = 0;
          }
          // save it for next time
          last_dl_tbs[dl_dci->harq_process][i] = dl_grant->mcs[i].tbs;
        }
      }
      // End CNI Fix
      switch(msg->format) {
        case SRSLTE_DCI_FORMAT0:
          ERROR("Error: no reason to be here\n");
          break;
        case SRSLTE_DCI_FORMAT1:
        case SRSLTE_DCI_FORMAT1A:
        case SRSLTE_DCI_FORMAT1C:
        case SRSLTE_DCI_FORMAT1B:
        case SRSLTE_DCI_FORMAT1D:
          fprintf(dci_file, "%04d\t%d\t%d\t1\t"
                          "%d\t%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\n", sfn, sf_idx, msg_rnti,
                  dl_grant->mcs[0].idx, dl_grant->nof_prb, dl_grant->mcs[0].tbs, -1, -1,
              msg->format+1, dl_dci->ndi, -1, dl_dci->harq_process,
              ncce, aggregation, cfi, prob);
          break;
        case SRSLTE_DCI_FORMAT2:
        case SRSLTE_DCI_FORMAT2A:
        case SRSLTE_DCI_FORMAT2B:
          fprintf(dci_file, "%04d\t%d\t%d\t1\t"
                          "%d\t%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\n", sfn, sf_idx, msg_rnti,
                  dl_grant->mcs[0].idx, dl_grant->nof_prb, dl_grant->mcs[0].tbs + dl_grant->mcs[1].tbs, dl_grant->mcs[0].tbs, dl_grant->mcs[1].tbs,
              msg->format+1, dl_dci->ndi, dl_dci->ndi_1, dl_dci->harq_process,
              ncce, aggregation, cfi, prob);
          break;
          //case SRSLTE_DCI_FORMAT3:
          //case SRSLTE_DCI_FORMAT3A:
        default:
          ERROR("Other formats\n");
      }
      ret = SRSLTE_SUCCESS;
    }
  }
  return ret;
}

bool falcon_dci_location_isvalid(falcon_dci_location_t *c) {
  if (c->L <= 3 && c->ncce <= 87) {
    return true;
  }
  else {
    return false;
  }
}
