#pragma once

#include "PhyCommon.h"
#include "MetaFormats.h"
#include "falcon/common/SubframeBuffer.h"
#include "falcon/phy/falcon_ue/falcon_ue_dl.h"

// handles a subframe
class SubframeWorker {
public:
  SubframeWorker(uint32_t idx,
                 uint32_t max_prb,
                 PhyCommon& common,
                 DCIMetaFormats& metaFormats);
  ~SubframeWorker();
  bool setCell(srslte_cell_t cell);
  void setRNTI(uint16_t rnti);
  void setChestCFOEstimateEnable(bool enable, uint32_t mask);
  void setChestAverageSubframe(bool enable);

  void prepare(uint32_t sf_idx, uint32_t sfn, bool updateMetaFormats);
  void work();
  void printStats();
  DCIBlindSearchStats& getStats();
  cf_t* getBuffer(uint32_t antenna_idx);
  cf_t** getBuffers() {return sfb.sf_buffer;}
  uint32_t getSfidx() const {return sf_idx;}
  uint32_t getSfn() const {return sfn;}

private:
  SubframeBuffer sfb;
  uint8_t *pch_payload_buffers[SRSLTE_MAX_CODEWORDS];

  uint32_t idx;
  uint32_t max_prb;
  PhyCommon& common;
  DCIMetaFormats& metaFormats;
  srslte_ue_dl_t ue_dl;
  uint32_t sf_idx;
  uint32_t sfn;
  bool updateMetaFormats;
  bool collision_dw, collision_up;
  DCIBlindSearchStats stats;
};
