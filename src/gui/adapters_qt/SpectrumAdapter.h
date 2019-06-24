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

#include "falcon/CCSnifferInterfaces.h"
#include "../model_dummy/ScanThread.h"
#include <QObject>

class SpectrumAdapter : public QObject, public Subscriber<ScanLineLegacy> {
  Q_OBJECT

public:
  SpectrumAdapter(QObject *parent = nullptr) : QObject(parent) {}
  virtual ~SpectrumAdapter() {}

  // Subscriber interface
  void update();
  void push(const ScanLineLegacy*) override;
  void notifyDetach();  // Forced detach

  bool emit_spectrum    = false;
  bool emit_difference  = false;
  bool emit_downlink    = false;
  bool emit_uplink      = false;
  bool emit_rnti_hist   = false;
  bool emit_perf_plot_a = false;
  bool emit_perf_plot_b = false;

signals:
  void update_ul(const ScanLineLegacy* line);
  void update_dl(const ScanLineLegacy* line);
  void update_spectrum(const ScanLineLegacy* line);
  void signal_update();
  void update_spectrum_diff(const ScanLineLegacy* line);
  void update_rnti_hist(const ScanLineLegacy* line);
  void update_perf_plot_a(const ScanLineLegacy* line);
  void update_perf_plot_b(const ScanLineLegacy* line);

};
