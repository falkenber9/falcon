
//#include "src/gui/plot_data.h" //MoHACKS

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>


#include "falcon/phy/falcon_phch/falcon_dci.h"
#include "falcon/phy/common/falcon_phy_common.h"

#include "srslte/phy/utils/debug.h"
#define ENABLE_DCI_LOGGING

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
                            "%d\t%d\t%d\t%d\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                    ul_grant->mcs.idx, ul_grant->L_prb, ul_grant->mcs.tbs, -1, -1,
                    ul_dci->ndi, (10*sfn+sf_idx)%8,
                    ncce, aggregation, cfi, histval);
#endif
          }
          else {
#ifdef ENABLE_DCI_LOGGING
            fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t0\t"
                            "%d\t%d\t%d\t%d\t%d\t"
                            "0\t%d\t-1\t%d\t"
                            "%d\t%d\t%d\t%d\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                    ul_grant->mcs.idx, ul_grant->L_prb, 0, -1, -1,
                    ul_dci->ndi, (10*sfn+sf_idx)%8,
                    ncce, aggregation, cfi, histval);
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
                          "%d\t%d\t%d\t%d\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                  dl_grant->mcs[0].idx, dl_grant->nof_prb, dl_grant->mcs[0].tbs, -1, -1,
              msg->format+1, dl_dci->ndi, -1, dl_dci->harq_process,
              ncce, aggregation, cfi, histval);
#endif
          break;
        case SRSLTE_DCI_FORMAT2:
        case SRSLTE_DCI_FORMAT2A:
        case SRSLTE_DCI_FORMAT2B:
#ifdef ENABLE_DCI_LOGGING
          fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t1\t"
                          "%d\t%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                  dl_grant->mcs[0].idx, dl_grant->nof_prb, dl_grant->mcs[0].tbs + dl_grant->mcs[1].tbs, dl_grant->mcs[0].tbs, dl_grant->mcs[1].tbs,
              msg->format+1, dl_dci->ndi, dl_dci->ndi_1, dl_dci->harq_process,
              ncce, aggregation, cfi, histval);
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

//#############################################MOHACKS################################

#define CNI_GUI

#ifdef CNI_GUI
// For Wrapper and data transfer to top layer
SRSLTE_API int srslte_dci_msg_to_trace_toTop(srslte_dci_msg_t *msg,
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
                   FILE* dci_file,
                   void *goal)
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
#ifdef ENABLE_DCI_LOGGING
          /*  fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t0\t"
                            "%d\t%d\t%d\t%d\t%d\t"
                            "0\t%d\t-1\t%d\t"
                            "%d\t%d\t%d\t%d\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                    ul_grant->mcs.idx, ul_grant->L_prb, ul_grant->mcs.tbs, -1, -1,
                    ul_dci->ndi, (10*sfn+sf_idx)%8,
                    ncce, aggregation, cfi, histval);*/
#endif
          }
          else {
#ifdef ENABLE_DCI_LOGGING
            /*fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t0\t"
                            "%d\t%d\t%d\t%d\t%d\t"
                            "0\t%d\t-1\t%d\t"
                            "%d\t%d\t%d\t%d\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                    ul_grant->mcs.idx, ul_grant->L_prb, 0, -1, -1,
                    ul_dci->ndi, (10*sfn+sf_idx)%8,
                    ncce, aggregation, cfi, histval);*/
#endif
          }
          //################### Mo Hacks

          call_function_2(goal, sfn, sf_idx, ul_grant->mcs.idx, ul_grant->mcs.tbs, ul_grant->L_prb);

          //###################
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
#ifdef ENABLE_DCI_LOGGING
          /*fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t1\t"
                          "%d\t%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                  dl_grant->mcs[0].idx, dl_grant->nof_prb, dl_grant->mcs[0].tbs, -1, -1,
              msg->format+1, dl_dci->ndi, -1, dl_dci->harq_process,
              ncce, aggregation, cfi, histval);
*/
          //################### Mo Hacks for Downlink

          call_function_3(goal, sfn, sf_idx, dl_grant->mcs[0].idx, dl_grant->mcs[0].tbs, dl_grant->nof_prb);

          //###################

#endif
          break;
        case SRSLTE_DCI_FORMAT2:
        case SRSLTE_DCI_FORMAT2A:
        case SRSLTE_DCI_FORMAT2B:
#ifdef ENABLE_DCI_LOGGING
       /*   fprintf(dci_file, "%ld.%06ld\t%04d\t%d\t%d\t1\t"
                          "%d\t%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\t"
                          "%d\t%d\t%d\t%d\n", timestamp.tv_sec, timestamp.tv_usec, sfn, sf_idx, msg_rnti,
                  dl_grant->mcs[0].idx, dl_grant->nof_prb, dl_grant->mcs[0].tbs + dl_grant->mcs[1].tbs, dl_grant->mcs[0].tbs, dl_grant->mcs[1].tbs,
              msg->format+1, dl_dci->ndi, dl_dci->ndi_1, dl_dci->harq_process,
              ncce, aggregation, cfi, histval);
*/
          //################### Mo Hacks for Downlink

          call_function_3(goal, sfn, sf_idx, dl_grant->mcs[0].idx, dl_grant->mcs[0].tbs, dl_grant->nof_prb);

          //###################
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


