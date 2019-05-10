#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "srslte/phy/phch/dci.h"

typedef enum ActivationReason {
  RM_ACT_UNSET = 0,
  RM_ACT_EVERGREEN,
  RM_ACT_RAR,
  RM_ACT_SHORTCUT,
  RM_ACT_HISTOGRAM,
  RM_ACT_OTHER
} rnti_manager_activation_reason_t;

typedef struct {
  uint16_t rnti;
  uint32_t frequency;
  uint32_t assoc_format_idx;
  rnti_manager_activation_reason_t reason;
  uint32_t last_seen;
} rnti_manager_active_set_t;

void* rnti_manager_create(uint32_t n_formats, uint32_t maxCandidatesPerStepPerFormat);
void rnti_manager_free(void* h);
void rnti_manager_add_evergreen(void* h, uint16_t rnti_start, uint16_t rnti_end, uint32_t format_idx);
void rnti_manager_add_candidate(void* h, uint16_t rnti, uint32_t format_idx);
int rnti_manager_validate(void* h, uint16_t rnti, uint32_t format_idx);
int rnti_manager_validate_and_refresh(void* h, uint16_t rnti, uint32_t format_idx);
void rnti_manager_activate_and_refresh(void* h, uint16_t rnti, uint32_t format_idx, rnti_manager_activation_reason_t reason);
void rnti_manager_step_time(void* h);
void rnti_manager_step_time_multi(void* h, uint32_t n_steps);
uint32_t rnti_manager_getFrequency(void* h, uint16_t rnti, uint32_t format_idx);
uint32_t rnti_manager_get_associated_format_idx(void* h, uint16_t rnti);
rnti_manager_activation_reason_t rnti_manager_get_activation_reason(void* h, uint16_t rnti);
void rnti_manager_get_histogram_summary(void* h, uint32_t* buf);
uint32_t rnti_manager_get_active_set(void* h, rnti_manager_active_set_t* buf, uint32_t buf_sz);
void rnti_manager_print_active_set(void* h);

const char* rnti_manager_activation_reason_string(rnti_manager_activation_reason_t reason);

#ifdef __cplusplus
}
#endif
