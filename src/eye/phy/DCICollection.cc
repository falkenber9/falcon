#include "DCICollection.h"
#include "DCIPrint.h"
#include "falcon/phy/falcon_phch/falcon_dci.h"

DCI_BASE::DCI_BASE() :
  rnti(FALCON_UNSET_RNTI),
  format(SRSLTE_DCI_NOF_FORMATS),
  nof_bits(0),
  hex(),
  location({0, 0}),
  histval(0)
{

}

DCI_DL::DCI_DL() :
  DCI_BASE(),
  dl_dci_unpacked(new srslte_ra_dl_dci_t),
  dl_grant(new srslte_ra_dl_grant_t)
{

}

DCI_UL::DCI_UL() :
  DCI_BASE(),
  ul_dci_unpacked(new srslte_ra_ul_dci_t),
  ul_grant(new srslte_ra_ul_grant_t)
{

}

DCICollection::DCICollection(const srslte_cell_t& cell) :
  cell(cell),
  sfn(0),
  sf_idx(0),
  cfi(0),
  timestamp(),
  dci_dl_collection(),
  dci_ul_collection(),
  rb_map_dl(cell.nof_prb, FALCON_UNSET_RNTI),
  rb_map_ul(cell.nof_prb, FALCON_UNSET_RNTI),
  dl_collision(false),
  ul_collision(false)
{
  dci_dl_collection.reserve(EXPECTED_NOF_DL_DCI_PER_SUBFRAME);
  dci_ul_collection.reserve(EXPECTED_NOF_UL_DCI_PER_SUBFRAME);
}

DCICollection::~DCICollection() {

}

void DCICollection::setTimestamp(timeval timestamp) {
  this->timestamp = timestamp;
}

timeval DCICollection::getTimestamp() const {
  return timestamp;
}

void DCICollection::setSubframe(uint32_t sfn, uint32_t sf_idx, uint32_t cfi) {
  this->sfn = sfn;
  this->sf_idx = sf_idx;
  this->cfi = cfi;
}

uint32_t DCICollection::get_sfn() const {
  return sfn;
}

uint32_t DCICollection::get_sf_idx() const {
  return sf_idx;
}

uint32_t DCICollection::get_cfi() const {
  return cfi;
}

const std::vector<DCI_DL>& DCICollection::getDCI_DL() const {
  return dci_dl_collection;
}

const std::vector<DCI_UL>& DCICollection::getDCI_UL() const {
  return dci_ul_collection;
}

const std::vector<uint16_t>& DCICollection::getRBMapDL() const {
  return rb_map_dl;
}

const std::vector<uint16_t>& DCICollection::getRBMapUL() const {
  return rb_map_ul;
}

bool DCICollection::hasCollisionDL() const {
  return dl_collision;
}

bool DCICollection::hasCollisionUL() const {
  return ul_collision;
}

void DCICollection::addCandidate(dci_candidate_t& cand,
                                 const srslte_dci_location_t& location,
                                 uint32_t histval) {
  DCI_DL* dci_dl = new DCI_DL();
  DCI_UL* dci_ul = new DCI_UL();

  char hex_str[SRSLTE_DCI_MAX_BITS/4 + 1];
  sprint_hex(hex_str, sizeof(hex_str), cand.dci_msg.data, cand.dci_msg.nof_bits);

#if 0 // Verify pack/unpack hex strings
  uint8_t dci_bits[SRSLTE_DCI_MAX_BITS];
  sscan_hex(hex_str, dci_bits, msg->nof_bits);
  for(int kk = 0; kk<msg->nof_bits; kk++) {
    if(dci_bits[kk] != msg->data[kk]) {
      ERROR("Mismatch in re-unpacked hex strings");
    }
  }
#endif

  //this function needs to be replaced by a more elegant code to unpack dci
  srslte_dci_msg_to_trace_timestamp(&cand.dci_msg,
                                    cand.rnti,
                                    cell.nof_prb,
                                    cell.nof_ports,
                                    dci_dl->dl_dci_unpacked.get(),
                                    dci_ul->ul_dci_unpacked.get(),
                                    dci_dl->dl_grant.get(),
                                    dci_ul->ul_grant.get(),
                                    sf_idx,
                                    sfn,
                                    histval,
                                    location.ncce,
                                    location.L,
                                    cand.dci_msg.format,
                                    cfi,
                                    0,
                                    timestamp,
                                    histval,
                                    nullptr);

  INFO("Collecting DCI for %d, %s, cce %d, L%d\n", cand.rnti, srslte_dci_format_string(cand.dci_msg.format), location.ncce, location.L);

  // Downlink
  if (cand.dci_msg.format != SRSLTE_DCI_FORMAT0) {
    dci_dl->rnti = cand.rnti;
    dci_dl->format = cand.dci_msg.format;
    dci_dl->nof_bits = cand.dci_msg.nof_bits;
    dci_dl->hex = hex_str;
    dci_dl->location = location;
    dci_dl->histval = histval;

    /* merge total RB map for RB allocation overview */
    for(uint32_t rb_idx = 0; rb_idx < cell.nof_prb; rb_idx++) {
      if(dci_dl->dl_grant->prb_idx[0][rb_idx] == true) {
        if(rb_map_dl[rb_idx] != FALCON_UNSET_RNTI) {
          dl_collision = true;
        }
        rb_map_dl[rb_idx] = cand.rnti;
      }
    }

    if(SRSLTE_VERBOSE_ISINFO()) {
      fprintf(stdout, "%d.%d: DL [", sfn, sf_idx);
      DCIPrint::printRBVectorColored(stdout, rb_map_dl);
      fprintf(stdout, "]\n");
    }

    //q.totRBdw += dci_dl->dl_grant->nof_prb;
    //q.totBWdw += (uint32_t)(dci_dl->dl_grant->mcs[0].tbs + dci_dl->dl_grant->mcs[1].tbs);
    //if (q.totRBdw > q.q->cell.nof_prb) q.totBWdw = q.q->cell.nof_prb;

    dci_dl_collection.push_back(std::move(*dci_dl));
  }

  // Uplink
  if (cand.dci_msg.format == SRSLTE_DCI_FORMAT0) {
    dci_ul->rnti = cand.rnti;
    dci_ul->format = cand.dci_msg.format;
    dci_ul->nof_bits = cand.dci_msg.nof_bits;
    dci_ul->hex = hex_str;
    dci_ul->location = location;
    dci_ul->histval = histval;

    /* merge total RB map for RB allocation overview */
    for(uint32_t rb_idx = 0; rb_idx < dci_ul->ul_grant->L_prb; rb_idx++) {
      if(rb_map_ul[dci_ul->ul_grant->n_prb[0] + rb_idx] != FALCON_UNSET_RNTI) {
        ul_collision = true;
      }
      rb_map_ul[dci_ul->ul_grant->n_prb[0] + rb_idx] = cand.rnti;
    }

    if(SRSLTE_VERBOSE_ISINFO()) {
      fprintf(stdout, "%d.%d: UL [", sfn, sf_idx);
      DCIPrint::printRBVectorColored(stdout, rb_map_ul);
      fprintf(stdout, "]\n");
    }

    //q.totRBup += dci_ul->ul_grant->L_prb;
    //q.totBWup +=  (uint32_t) dci_ul->ul_grant->mcs.tbs;
    //if (q.totRBup > q.q->cell.nof_prb) q.totBWup = q.q->cell.nof_prb;

    dci_ul_collection.push_back(std::move(*dci_ul));
  }

  delete dci_dl;
  delete dci_ul;
}

std::vector<uint16_t> DCICollection::applyLegacyColorMap(const std::vector<uint16_t>& RBMap) {
  std::vector<uint16_t> result(RBMap);
  for(std::vector<uint16_t>::iterator it = result.begin(); it != result.end(); ++it) {
    if(*it != FALCON_UNSET_RNTI) {
      uint16_t color = *it;
      color = (((color & 0xFF00) >> 8) | ((color & 0x00FF) << 8));
      color = (((color & 0xF0F0) >> 4) | ((color & 0x0F0F) << 4));
      color = (((color & 0xCCCC) >> 2) | ((color & 0x3333) << 2));
      color = (((color & 0xAAAA) >> 1) | ((color & 0x5555) << 1));
      color = (color >> 1) + 16000;
      *it = color;
    }
  }
  return result;
}
