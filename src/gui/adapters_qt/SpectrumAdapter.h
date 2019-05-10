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
  bool emit_plot1       = false;

signals:
  void update_ul(const ScanLineLegacy* line);
  void update_dl(const ScanLineLegacy* line);
  void update_spectrum(const ScanLineLegacy* line);
  void signal_update();
  void update_spectrum_diff(const ScanLineLegacy* line);
  void update_rnti_hist(const ScanLineLegacy* line);

};
