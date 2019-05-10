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
