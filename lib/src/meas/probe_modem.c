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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "falcon/meas/probe_modem.h"

#include "srslte/phy/common/phy_common.h"

#include "cmnalib/traffic_curl.h"
#include "cmnalib/traffic_types.h"

#include "cmnalib/logger.h"

#include "cmnalib/at_sierra_wireless_em7565.h"

#define PROGRESS_CALLBACK_CONTINUE 0
#define PROGRESS_CALLBACK_ERROR -1
#define PROGRESS_CALLBACK_ABORT -2

typedef enum program_state {
  STATE_NORMAL_OPERATION = 0,
  STATE_FAILURE_RESUME,
  STATE_FAILURE_EXIT,
  STATE_FINISH,
} program_state_t;

bool isZero(double value) {
  const double epsilon = 1e-5;
  return (value < epsilon) && (value > -epsilon);
}

/**
  Container structure for passing access/handles to callback functions
  which are frequently called by the traffic-generator during transfer
*/
typedef struct progress_callback_context {
  volatile transfer_state_t state;
  double datarate_dl;
  double datarate_ul;
  double total_transfer_time;
  //    modem_status_t* modem_status;
  //    trace_handle_t* trace_handle;
  //    int trace_transmission_counter;
  //    double old_latitude;
  //    double old_longitude;
  //    double total_distance;
  const probe_event_handlers_t* ev_funcs;
  void* ev_param;
} progress_callback_context_t;

int progress_callback(void* void_context, transfer_statusreport_t* status) {
  progress_callback_context_t* context = (progress_callback_context_t*)void_context;
  if(context != NULL && status != NULL) {
    context->datarate_ul = status->datarate_ul;
    context->datarate_dl = status->datarate_dl;
    context->total_transfer_time = status->total_transfer_time;

    if(context->ev_funcs != NULL && context->ev_funcs != NULL) {
      context->ev_funcs->action_during_transfer(context->ev_param);
    }

    if(context->state == TS_CANCELED) {
      return PROGRESS_CALLBACK_ABORT;
    }
  }
  else {
    ERROR("context or status not set\n");
    return PROGRESS_CALLBACK_ERROR;
  }
  return PROGRESS_CALLBACK_CONTINUE;
}

progress_callback_context_t* init_progress_callback_context() {
  progress_callback_context_t* context = calloc(1, sizeof(progress_callback_context_t));

  context->datarate_dl = 0;
  context->datarate_ul = 0;
  context->total_transfer_time = 0;
  context->state = TS_IDLE;
  context->ev_funcs = NULL;
  context->ev_param = NULL;

  return context;
}

void release_progress_callback_context(progress_callback_context_t* context) {
  free(context);
}

typedef enum transfer_type {
  TS_TYPE_UPLINK = 0,
  TS_TYPE_DOWNLINK,
} transfer_type_t;

typedef struct datatransfer_context {
  char* url;
  size_t payload_size;
  transfer_type_t type;
  const probe_event_handlers_t* ev_funcs;
  void* ev_param;
  progress_callback_context_t* progress_context;
} datatransfer_context_t;

struct datatransfer_thread {
  pthread_t* thread;
  datatransfer_context_t* context;
}; //datatransfer_thread_t;

void* datatransfer_main(void* void_context) {
  datatransfer_context_t* context = (datatransfer_context_t*)void_context;
  int ret = 0;

  if(context->ev_funcs != NULL && context->ev_funcs != NULL) {
    context->ev_funcs->action_before_transfer(context->ev_param);
  }

  context->progress_context->state = TS_TRANSMITTING;
  switch(context->type) {
    case TS_TYPE_UPLINK:
      ret = tc_upload_randomdata(context->url, (size_t)context->payload_size, &progress_callback, context->progress_context, 1);
      break;
    case TS_TYPE_DOWNLINK:
      ret = tc_download_and_discard(context->url, (size_t)context->payload_size, &progress_callback, context->progress_context, 1);
      break;
    default:
      ret = -1;
  }
  if(ret == 0) {  /* no error */
    if(context->progress_context->state != TS_CANCELED) {
      context->progress_context->state = TS_FINISHED;
    }
  }
  else {  /* transfer failure */
    context->progress_context->state = TS_FAILED;
  }

  if(context->ev_funcs != NULL && context->ev_funcs != NULL) {
    context->ev_funcs->action_after_transfer(context->ev_param);
  }

  return NULL;
}

datatransfer_thread_t* xlink_probe(size_t payload_size, const char *url, const probe_event_handlers_t* ev_funcs, void* ev_param, transfer_type_t type) {
  datatransfer_thread_t* dtt  = calloc(1, sizeof(datatransfer_thread_t));
  dtt->context = calloc(1, sizeof(datatransfer_context_t));
  dtt->thread = calloc(1, sizeof(pthread_t));

  dtt->context->payload_size = payload_size;
  dtt->context->url = calloc(1, strlen(url)+1);
  dtt->context->type = type;
  dtt->context->ev_funcs = ev_funcs;
  dtt->context->ev_param = ev_param;
  strcpy(dtt->context->url, url);
  dtt->context->progress_context = init_progress_callback_context();
  dtt->context->progress_context->ev_funcs = dtt->context->ev_funcs;
  dtt->context->progress_context->ev_param = dtt->context->ev_param;

  if(pthread_create(dtt->thread, NULL, datatransfer_main, (void*)dtt->context)) {
    ERROR("Could not create datatransfer thread");
    abort();
  }

  return dtt;
}

datatransfer_thread_t* uplink_probe(size_t payload_size, const char *url, const probe_event_handlers_t* ev_funcs, void* ev_param) {
  return xlink_probe(payload_size, url, ev_funcs, ev_param, TS_TYPE_UPLINK);
}

datatransfer_thread_t* downlink_probe(size_t max_payload_size, const char *url, const probe_event_handlers_t* ev_funcs, void* ev_param) {
  return xlink_probe(max_payload_size, url, ev_funcs, ev_param, TS_TYPE_DOWNLINK);
}

void get_probe_status(probe_result_t* result, datatransfer_thread_t* datatransfer_thread) {
  if(result != NULL && datatransfer_thread != NULL) {
    result->state = datatransfer_thread->context->progress_context->state;
    result->datarate_dl = datatransfer_thread->context->progress_context->datarate_dl;
    result->datarate_ul = datatransfer_thread->context->progress_context->datarate_ul;
    result->total_transfer_time = datatransfer_thread->context->progress_context->total_transfer_time;
    result->payload_size = datatransfer_thread->context->payload_size;
  }
}

void release_probe(datatransfer_thread_t* dtt) {
  if(dtt != NULL) {
    pthread_join(*dtt->thread, NULL);

    free(dtt->thread);
    release_progress_callback_context(dtt->context->progress_context);
    free(dtt->context->url);
    free(dtt->context);
    free(dtt);
  }

}

void cancel_transfer(datatransfer_thread_t* dtt) {
  if(dtt != NULL &&
     dtt->context != NULL &&
     dtt->context->progress_context != NULL) {
    dtt->context->progress_context->state = TS_CANCELED;
  }
}

struct modem {
  sw_em7565_t* modem;
  pthread_mutex_t mutex;

}; // modem_t

modem_t* init_modem() {
  modem_t* m = calloc(1, sizeof(modem_t));
  pthread_mutex_init (&m->mutex, NULL);

  // Open modem interface
  m->modem = sw_em7565_init_first();
  if(m->modem == NULL) {
    ERROR("Could not initialize modem\n");
    free(m);
    return NULL;
  }
  DEBUG("Opened modem\n");

  return m;
}

void release_modem(modem_t* m) {
  if(m != NULL && m->modem != NULL) {
    sw_em7565_destroy(m->modem);
    m->modem = NULL;
    pthread_mutex_destroy(&m->mutex);
  }
  free(m);
}

int _is_online_modem(modem_t* m) {
  int int_ret = sw_em7565_get_data_connection(m->modem);
  if(int_ret == DATA_CONNECTION_STATUS_ERROR) {
    WARNING("Failed to get data connection status\n");
  }
  return int_ret == DATA_CONNECTION_STATUS_ENABLED;
}

int is_online_modem(modem_t* m) {
  pthread_mutex_lock(&m->mutex);
  int int_ret = _is_online_modem(m);
  pthread_mutex_unlock(&m->mutex);
  return int_ret;
}

int _set_online_modem(modem_t* m, int value) {
  int state = _is_online_modem(m);
  if((value != 0 && state != 0) || (value == 0 && state == 0)) {
    // state already equals to value
    return 1;
  }
  else {
    // change state
    sw_response_t ret;
    if(value) {
      ret = sw_em7565_set_data_connection(m->modem, DATA_CONNECTION_STATUS_ENABLED);
    }
    else {
      ret = sw_em7565_set_data_connection(m->modem, DATA_CONNECTION_STATUS_DISABLED);
    }

    if(ret > SW_RESPONSE_SUCCESS) {
      ERROR("Could not enable data connection\n");
      return 0;
    }
    else {
      return 1;
    }
  }
}

int set_online_modem(modem_t* m, int value) {
  pthread_mutex_lock(&m->mutex);
  int int_ret = _set_online_modem(m, value);
  pthread_mutex_unlock(&m->mutex);
  return int_ret;
}

int configure_modem(modem_t* m) {
  pthread_mutex_lock(&m->mutex);
  sw_response_t ret;

  // Init data connection

  //ret = sw_em7565_set_APN(modem, APN_SLOT_DEFAULT, APN_IP_VERSION_ANY, "internet.telekom");
  //if(ret != RESPONSE_OK) {
  //    WARNING("Failed to setup APN, continue\n");
  //}

  //Set radio access type (RAT) to LTE only if necessary
  sw_em7565_radio_access_type_t rat = SW_RAT_AUTOMATIC;
  ret = sw_em7565_get_radio_access_type(m->modem, &rat);
  if(ret > SW_RESPONSE_SUCCESS) {
    WARNING("Could not read radio access type\n");
  }
  if(rat != SW_RAT_LTE_ONLY) {
    ret = sw_em7565_set_radio_access_type(m->modem, SW_RAT_LTE_ONLY);
    if(ret > SW_RESPONSE_SUCCESS) {
      WARNING("Could not set radio access type to LTE only\n");
    }
  }

//  //Enable data connection if not already done
//  int_ret = sw_em7565_get_data_connection(m->modem);
//  if(int_ret == DATA_CONNECTION_STATUS_ERROR) {
//    WARNING("Failed to get data connection status\n");
//  }
//  if(int_ret != DATA_CONNECTION_STATUS_ENABLED) {
//    // try to establish data connection if disabled or error
//    ret = sw_em7565_set_data_connection(m->modem, DATA_CONNECTION_STATUS_ENABLED);
//    if(ret > SW_RESPONSE_SUCCESS) {
//      ERROR("Could not establish data connection\n");
//      pthread_mutex_unlock(&m->mutex);
//      return 0;
//    }
//    DEBUG("Data connection established\n");
//  }

  // Restart ethernet interface
  //#define RESTART_INTERFACE
#ifdef RESTART_INTERFACE
  set_if_down(INTERFACE_NAME, 0);
  sleep(2);
  set_if_up(INTERFACE_NAME, 0);
#endif

  pthread_mutex_unlock(&m->mutex);
  return 1;
}

sw_em7565_gstatus_response_t* alloc_gstatus() {
  return sw_em7565_allocate_status();
}

void release_gstatus(sw_em7565_gstatus_response_t* gstatus) {
  if(gstatus != NULL) {
    sw_em7565_free_status(gstatus);
    gstatus = NULL;
  }
}

int modem_get_gstatus(modem_t* m, sw_em7565_gstatus_response_t* gstatus) {
  if(m == NULL) {
    ERROR("Invalid argument\n");
    return 0;
  }

  pthread_mutex_lock(&m->mutex);

  sw_response_t ret;

  //gettimeofday(&t_start, NULL);

  // Read network information from modem
  ret = sw_em7565_get_status(m->modem, gstatus);
  if(ret >= SW_RESPONSE_CRITICAL) {
    ERROR("Failed to read modem status\n");
    pthread_mutex_unlock(&m->mutex);
    return 0;
  }

  pthread_mutex_unlock(&m->mutex);
  return 1;
}

network_info_t* alloc_network_info_shallow() {
  network_info_t* result = NULL;
  result = calloc(1, sizeof(network_info_t));
  if(result != NULL) {
    result->gstatus = NULL;
    result->lteinfo = NULL;
  }
  return result;
}

network_info_t* alloc_network_info() {
  network_info_t* result = NULL;
  result = alloc_network_info_shallow();
  if(result != NULL) {
    result->gstatus = sw_em7565_allocate_status();
    result->lteinfo = sw_em7565_allocate_lteinfo();
  }
  return result;
}

int modem_get_network_info(modem_t* m, network_info_t* netinfo) {
  if(m == NULL) {
    ERROR("Invalid argument\n");
    return 0;
  }

  pthread_mutex_lock(&m->mutex);

  sw_response_t ret;

  //gettimeofday(&t_start, NULL);

  // Read network information from modem
  ret = sw_em7565_get_status(m->modem, netinfo->gstatus);
  if(ret >= SW_RESPONSE_CRITICAL) {
    ERROR("Failed to read modem status\n");
    pthread_mutex_unlock(&m->mutex);
    return 0;
  }
  ret = sw_em7565_get_lteinfo(m->modem, netinfo->lteinfo);
  if(ret >= SW_RESPONSE_CRITICAL) {
    ERROR("Failed to read lteinfo from modem\n");
    pthread_mutex_unlock(&m->mutex);
    return 0;
  }

  // Compute required values
  netinfo->nof_prb = (uint32_t)netinfo->gstatus->lte_bw_MHz * 5;
  netinfo->rf_freq = (double)srslte_band_fd((uint32_t)netinfo->lteinfo->earfn);
  netinfo->N_id_2 = netinfo->lteinfo->pci % 3;

  pthread_mutex_unlock(&m->mutex);
  return 1;
}

void release_network_info(network_info_t* info) {
  if(info != NULL) {
    if(info->gstatus != NULL) {
      sw_em7565_free_status(info->gstatus);
      info->gstatus = NULL;
    }
    if(info->lteinfo != NULL) {
      sw_em7565_free_lteinfo(info->lteinfo);
      info->lteinfo = NULL;
    }
    free(info);
  }
}

int network_info_is_equal(const network_info_t* info1, const network_info_t* info2) {
  if(info1 == NULL || info2 == NULL) return 0;
  //if(info1->N_id_2 != info2->N_id_2) return 0;
  //if(info1->nof_prb != info2->nof_prb) return 0;
  if(!isZero(info1->rf_freq - info2->rf_freq)) return 0;
  if(info1->lteinfo->pci != info2->lteinfo->pci) return 0;
  if(info1->lteinfo->cid != info2->lteinfo->cid) return 0;
  if(info1->lteinfo->tac != info2->lteinfo->tac) return 0;
  return 1;
}

operator_name_t* alloc_operator_name() {
  return calloc(1, sizeof(operator_name_t));
}

operator_name_t* modem_get_operator_name(modem_t* m) {
  operator_name_t* result = alloc_operator_name();
  pthread_mutex_lock(&m->mutex);
  sw_response_t ret;
  ret = sw_em7565_get_current_operator(m->modem, result->name);
  if(ret >= SW_RESPONSE_CRITICAL) {
    ERROR("Failed to read current operator\n");
    release_operator_name(result);
    pthread_mutex_unlock(&m->mutex);
    return NULL;
  }
  pthread_mutex_unlock(&m->mutex);
  return result;
}
void release_operator_name(operator_name_t* name) {
  if(name) {
    sw_em7565_free_current_operator(name->name);
    name->name = NULL;
  }
  free(name);
}

void modem_enable_logger(int enable) {
  enable_logger = enable;
}

const char *transfer_state_to_string(transfer_state_t state) {
  switch (state) {
    case TS_UNDEFINED:
      return "Undefined";
    case TS_IDLE:
      return "Idle";
    case TS_TRANSMITTING:
      return "Transmitting";
    case TS_FINISHED:
      return "Finished";
    case TS_CANCELED:
      return "Canceled";
    case TS_FAILED:
      return "Failed";
    default:
      return "MissingString";
  }
}

transfer_state_t string_to_transfer_state(const char* str) {
  if(strcmp(str, "Idle") == 0) {
    return TS_IDLE;
  }
  if(strcmp(str, "Transmitting") == 0) {
    return TS_TRANSMITTING;
  }
  if(strcmp(str, "Finished") == 0) {
    return TS_FINISHED;
  }
  if(strcmp(str, "Canceled") == 0) {
    return TS_CANCELED;
  }
  if(strcmp(str, "Failed") == 0) {
    return TS_FAILED;
  }

  return TS_UNDEFINED;
}

int start_gps(modem_t* m) {
  pthread_mutex_lock(&m->mutex);
  sw_response_t ret;

  //Set Antenna power (if required)
  if(sw_em7565_get_antenna_power(m->modem) != GPS_ANTENNA_POWER_3V) {
    sw_em7565_set_antenna_power(m->modem, GPS_ANTENNA_POWER_3V);
  }

  //Start GPS (if required)
  sw_em7565_gps_status_t gps_status;
  ret = sw_em7565_gps_status(m->modem, &gps_status);
  if(ret != SW_RESPONSE_SUCCESS) {
    ERROR("Could not read GPS status\n");
    pthread_mutex_unlock(&m->mutex);
    return -1;
  }
  if(gps_status.fix_session_status != GPS_STATUS_ACTIVE) {
     ret = sw_em7565_start_gps_default(m->modem);
     if(ret != SW_RESPONSE_SUCCESS) {
       ERROR("Could not start GPS session\n");
       pthread_mutex_unlock(&m->mutex);
       return -1;
     }
  }

  pthread_mutex_unlock(&m->mutex);
  return 0;
}

int get_gps_fix(modem_t* m, sw_em7565_gpsloc_response_t* location) {
  pthread_mutex_lock(&m->mutex);
  sw_response_t ret;
  ret = sw_em7565_get_gpsloc(m->modem, location);
  if(ret != SW_RESPONSE_SUCCESS) {
    ERROR("Could not get GPS location\n");
    pthread_mutex_unlock(&m->mutex);
    return -1;
  }

  pthread_mutex_unlock(&m->mutex);
  return 0;
}

int stop_gps(modem_t* m) {
  pthread_mutex_lock(&m->mutex);
  sw_response_t ret;
  ret = sw_em7565_stop_gps(m->modem);
  if(ret != SW_RESPONSE_SUCCESS) {
    ERROR("Could not stop GPS\n");
    pthread_mutex_unlock(&m->mutex);
    return -1;

  }
  pthread_mutex_unlock(&m->mutex);
  return 0;
}

