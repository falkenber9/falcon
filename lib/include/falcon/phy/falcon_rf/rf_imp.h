#pragma once

#include "srslte/srslte.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "srslte/phy/rf/rf.h"

int srslte_rf_kill_gain_thread(srslte_rf_t *rf);
int falcon_rf_recv_wrapper(void *h, cf_t *data[SRSLTE_MAX_PORTS], uint32_t nsamples, srslte_timestamp_t *t);
double falcon_rf_set_rx_gain_th_wrapper(void *h, double f);

#ifdef __cplusplus
}
#endif
