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
#ifndef CNI_DECODER_H
#define CNI_DECODER_H
#pragma once




#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

extern void *decoderthread;  // Pointer auf decoderthread Funktionen
extern volatile bool go_exit;

typedef struct {
  int nof_subframes;
  int cpu_affinity;
  bool disable_plots;
  bool disable_cfo;
  uint32_t time_offset;
  int force_N_id_2;
  char *input_file_name;
  char *rnti_list_file;
  char *rnti_list_file_out;
  int file_offset_time;
  float file_offset_freq;
  uint32_t file_nof_prb;
  uint32_t file_nof_ports;
  uint32_t file_cell_id;
  char *rf_args;
  uint32_t rf_nof_rx_ant;
  double rf_freq;
  float rf_gain;
  int net_port;
  char *net_address;
  int net_port_signal;
  char *net_address_signal;
  int decimate;
} prog_args_t;


void plot_scanLines(void *f_pointer, float *data_ul, float *data_dl, float *data_spectrum, uint32_t *rnti_hist);

//extern volatile prog_args_t prog_args;

//extern volatile bool go_exit;


int start_cni_decoder();

#ifdef __cplusplus
}
#endif


#endif
