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
#include "falcon/phy/falcon_rf/rf_imp.h" 

#include <pthread.h>
#include <signal.h>

int srslte_rf_kill_gain_thread(srslte_rf_t *rf) {
  if(rf->thread_gain) {
    if(!pthread_kill(rf->thread_gain, 0)) {
       pthread_cancel(rf->thread_gain);
       pthread_join(rf->thread_gain, NULL);
    }
    rf->thread_gain = 0;
  }
  return 0;
}

int falcon_rf_recv_wrapper(void *h, cf_t *data[SRSLTE_MAX_PORTS], uint32_t nsamples, srslte_timestamp_t *t) {
  (void)t;  //unused
  DEBUG(" ----  Receive %d samples  ---- \n", nsamples);
  void *ptr[SRSLTE_MAX_PORTS];
  for (int i=0;i<SRSLTE_MAX_PORTS;i++) {
    ptr[i] = data[i];
  }
  return srslte_rf_recv_with_time_multi((srslte_rf_t*)(h), ptr, nsamples, true, NULL, NULL);
}

double falcon_rf_set_rx_gain_th_wrapper(void *h, double f) {
  return srslte_rf_set_rx_gain_th((srslte_rf_t*)(h), f);
}
