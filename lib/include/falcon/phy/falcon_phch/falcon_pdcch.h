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

#pragma once

#include "falcon/phy/common/falcon_phy_common.h"
#include "falcon/phy/falcon_phch/falcon_dci.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "srslte/phy/phch/pdcch.h"

//#define PWR_THR				.7f >> moved to falcon_dci.h

//#define MAX_NUM_OF_CCE 64
#define MAX_NUM_OF_CCE 84

SRSLTE_API uint32_t srslte_pdcch_nof_cce(srslte_pdcch_t *q, uint32_t cfi);

SRSLTE_API float srslte_pdcch_decode_msg_check_power(srslte_pdcch_t *q,
                                          uint32_t cfi,
                                          srslte_dci_msg_t *msg,
                                          falcon_dci_location_t *location,
                                          srslte_dci_format_t format,
                                          uint16_t *crc_rem,
                                          uint16_t *list);

/* Decoding functions: Try to decode a DCI message after calling srslte_pdcch_extract_llr */
SRSLTE_API float srslte_pdcch_decode_msg_check(srslte_pdcch_t *q,
                                               uint32_t cfi,
                                               srslte_dci_msg_t *msg,
                                               falcon_dci_location_t *location,
                                               srslte_dci_format_t format,
                                               uint16_t *crc_rem,
                                               uint16_t *list);

/* Decoding functions: Try to decode a DCI message after calling srslte_pdcch_extract_llr */
SRSLTE_API int srslte_pdcch_decode_msg_limit_avg_llr_power(srslte_pdcch_t *q,
                                                           srslte_dci_msg_t *msg,
                                                           falcon_dci_location_t *location,
                                                           srslte_dci_format_t format,
                                                           uint32_t cfi,
                                                           uint16_t *crc_rem,
                                                           double minimum_avg_llr_bound);

/* Decoding functions: Try to decode a DCI message after calling srslte_pdcch_extract_llr */
SRSLTE_API int srslte_pdcch_decode_msg_no_llr_limit(srslte_pdcch_t *q,
                                                    srslte_dci_msg_t *msg,
                                                    srslte_dci_location_t *location,
                                                    srslte_dci_format_t format,
                                                    uint16_t *crc_rem);

SRSLTE_API uint32_t srslte_pdcch_nof_missed_cce_compat(falcon_dci_location_t *c,
                                           uint32_t nof_locations,
                                           uint32_t cfi);

SRSLTE_API uint32_t srslte_pdcch_nof_missed_cce(srslte_pdcch_t *q, uint32_t cfi,
                                           falcon_cce_to_dci_location_map_t *cce_map,
                                           uint32_t max_cce);

/* Calculate average power (LLR) on each CCE for DCI filtering */
SRSLTE_API int srslte_pdcch_cce_avg_llr_power(srslte_pdcch_t *q, uint32_t cfi,
                                              falcon_cce_to_dci_location_map_t *cce_map,
                                              uint32_t max_cce);


/* Function for generation of UE-specific and common search space DCI locations according to rnti */
SRSLTE_API uint32_t srslte_pdcch_generic_locations_ncce(uint32_t nof_cce,
                                                        srslte_dci_location_t *c,
                                                        uint32_t max_candidates,
                                                        uint32_t nsubframe, uint16_t rnti);

/* Validation function for DCI candidates */
SRSLTE_API uint32_t srslte_pdcch_validate_location(uint32_t nof_cce, uint32_t ncce, uint32_t l,
                                                   uint32_t nsubframe, uint16_t rnti);

SRSLTE_API uint32_t srslte_pdcch_ue_locations_all(srslte_pdcch_t *q,
                                                  falcon_dci_location_t *locations,
                                                  uint32_t max_locations,
                                                  uint32_t nsubframe,
                                                  uint32_t cfi);

SRSLTE_API uint32_t srslte_pdcch_ue_locations_all_map(srslte_pdcch_t *q,
                                                      falcon_dci_location_t *c,
                                                      uint32_t max_candidates,
                                                      falcon_cce_to_dci_location_map_t *cce_map,
                                                      uint32_t max_cce,
                                                      uint32_t nsubframe, uint32_t cfi);

SRSLTE_API uint32_t srslte_pdcch_uncheck_ue_locations(falcon_dci_location_t *c,
                                                uint32_t nof_locations);

SRSLTE_API int srslte_pdcch_decode_msg_power(srslte_pdcch_t *q,
					     srslte_dci_msg_t *msg,
					     srslte_dci_location_t *location,
					     uint32_t sfn,
					     uint32_t sf_idx);

//SRSLTE_API uint32_t srslte_pdcch_ue_locations_ncce_check(uint32_t nof_cce,
//							 uint32_t nsubframe,
//							 uint16_t rnti,
//							 uint32_t this_ncce);
SRSLTE_API uint32_t srslte_pdcch_ue_locations_check(srslte_pdcch_t *q,
						    uint32_t nsubframe,
						    uint32_t cfi,
						    uint16_t rnti,
						    uint32_t this_ncce);


#ifdef __cplusplus
}
#endif
