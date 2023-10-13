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

#include "cmnalib/at_sierra_wireless_em7565.h"

#ifdef __cplusplus
extern "C" {
#endif


//#define DEFAULT_URL "mptcp1.pi21.de:5002"
#define DEFAULT_URL "www.kn.e-technik.tu-dortmund.de"

typedef enum transfer_state {
  TS_UNDEFINED = 0,   // just initialized or incomplete handle
  TS_IDLE,            // preparing transfer
  TS_TRANSMITTING,    // active transmission
  TS_FINISHED,        // finished successfully
  TS_CANCELED,        // canceled by some reason
  TS_FAILED,          // failure, e.g. unreachable URL
} transfer_state_t;

typedef struct probe_result {
  transfer_state_t state;
  double datarate_dl;
  double datarate_ul;
  double total_transfer_time;
  size_t payload_size;
} probe_result_t;


typedef void (*event_handler_function)(void*);
typedef struct probe_event_handlers {
  event_handler_function action_before_transfer;
  event_handler_function action_during_transfer;
  event_handler_function action_after_transfer;
} probe_event_handlers_t;

// Types and methods for transmissions
typedef struct datatransfer_thread datatransfer_thread_t;
datatransfer_thread_t* uplink_probe(size_t payload_size, const char *url, const probe_event_handlers_t* ev_funcs, void* ev_param);
datatransfer_thread_t* downlink_probe(size_t max_payload_size, const char *url, const probe_event_handlers_t* ev_funcs, void* ev_param);
void release_probe(datatransfer_thread_t* dtt);
void get_probe_status(probe_result_t* result, datatransfer_thread_t* datatransfer_thread);
void cancel_transfer(datatransfer_thread_t* dtt);
const char* transfer_state_to_string(transfer_state_t state);
transfer_state_t string_to_transfer_state(const char* str);


// Types and methods for modem interaction
typedef struct modem modem_t;
modem_t* init_modem();
void release_modem(modem_t* m);
int configure_modem(modem_t* m);
int is_online_modem(modem_t* m);
int set_online_modem(modem_t* m, int value);



// Network info
typedef struct network_info {
  uint32_t nof_prb;
  int N_id_2;
  double rf_freq;
  // timestamp?
  sw_em7565_lteinfo_response_t* lteinfo;
  sw_em7565_gstatus_response_t* gstatus;
} network_info_t;

sw_em7565_gstatus_response_t* alloc_gstatus();
void release_gstatus(sw_em7565_gstatus_response_t* gstatus);
int modem_get_gstatus(modem_t* m, sw_em7565_gstatus_response_t* gstatus);

network_info_t* alloc_network_info_shallow();
network_info_t* alloc_network_info();
int modem_get_network_info(modem_t* m, network_info_t* netinfo);
void release_network_info(network_info_t* info);
int network_info_is_equal(const network_info_t *info1, const network_info_t *info2);

// Operator name
typedef struct operator_name {
  sw_em7565_current_operator_t* name;
} operator_name_t;
operator_name_t* alloc_operator_name();
operator_name_t* modem_get_operator_name(modem_t* m);
void release_operator_name(operator_name_t* name);

// GPS
int start_gps(modem_t* m);
int get_gps_fix(modem_t* m, sw_em7565_gpsloc_response_t* location);
int stop_gps(modem_t* m);

void modem_enable_logger(int enable);

#ifdef __cplusplus
}
#endif
