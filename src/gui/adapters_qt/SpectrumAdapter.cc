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
#include "SpectrumAdapter.h"


void SpectrumAdapter::update() {
  // currently, only push() is implemented

  emit signal_update();

  // emit signals to gui here
}

void SpectrumAdapter::push(const ScanLineLegacy* data) {
  //ScanLine* scanline = dynamic_cast<ScanLine*>(data);

  switch (data->type) {
    case(ScanLineType_t::SCAN_LINE_UPLINK):
      if(emit_uplink) emit update_ul(data);
      else delete data;
      break;
    case(ScanLineType_t::SCAN_LINE_DOWNLINK):
      if(emit_downlink) emit update_dl(data);
      else delete data;
      break;
    case(ScanLineType_t::SCAN_LINE_SPECTRUM):
      if(emit_spectrum) emit update_spectrum(data);
      else delete data;
      break;
    case(ScanLineType_t::SCAN_LINE_SPECTRUM_DIFF):
      if(emit_difference) emit update_spectrum_diff(data);
      else delete data;
      break;
    case(ScanLineType_t::SCAN_LINE_RNTI_HIST):
      if(emit_rnti_hist) emit update_rnti_hist(data);
      else delete data;
      break;
    case(ScanLineType_t::SCAN_LINE_PERF_PLOT_A):
      if(emit_perf_plot_a) emit update_perf_plot_a(data);
      else delete data;
      break;
    case(ScanLineType_t::SCAN_LINE_PERF_PLOT_B):
      if(emit_perf_plot_b) emit update_perf_plot_b(data);
      else delete data;
      break;
  }
}

void SpectrumAdapter::notifyDetach() {

}
