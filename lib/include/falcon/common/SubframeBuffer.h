#pragma once

#include <stdint.h>
#include "srslte/srslte.h"

struct SubframeBuffer {
  SubframeBuffer(uint32_t rf_nof_rx_ant);
  SubframeBuffer(const SubframeBuffer&) = delete; //prevent copy
  SubframeBuffer& operator=(const SubframeBuffer&) = delete; //prevent copy
  ~SubframeBuffer();
  const uint32_t rf_nof_rx_ant;
  cf_t *sf_buffer[SRSLTE_MAX_PORTS] = {nullptr};
};
