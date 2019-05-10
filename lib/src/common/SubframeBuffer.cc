#include "falcon/common/SubframeBuffer.h"

SubframeBuffer::SubframeBuffer(uint32_t rf_nof_rx_ant) : rf_nof_rx_ant(rf_nof_rx_ant) {
  for (uint32_t i = 0; i < rf_nof_rx_ant; i++) {
    sf_buffer[i] = static_cast<cf_t*>(srslte_vec_malloc(3*static_cast<uint32_t>(sizeof(cf_t))*static_cast<uint32_t>(SRSLTE_SF_LEN_PRB(100))));
  }
}

SubframeBuffer::~SubframeBuffer() {
  for (uint32_t i = 0; i < rf_nof_rx_ant; i++) {
    free(sf_buffer[i]);
    sf_buffer[i] = nullptr;
  }
}
