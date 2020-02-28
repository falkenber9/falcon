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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <mutex>
#include <QMainWindow>
#include "spectrum.h"
#include "falcon/CCSnifferInterfaces.h"
#include "adapters_qt/SpectrumAdapter.h"

#include "model/EyeThread.h"
#include "model_dummy/cni_cc_decoderThread.h"

#include "QTextBrowser"
#include "stdio.h"
#include "QtCharts"
#include "QtCharts/QLineSeries"
#include "settings.h"
#include "plots.h"
#include "performance_plot.h"
#include "waterfall.h"
#include "rnti_table.h"
#include "colorpicker.h"
#include <QColorDialog>

#include "qcustomplot/qcustomplot.h"
#include "rangewidget/RangeWidget.h"

#include "model_dummy/cni_cc_decoder.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:

  void on_actionStart_triggered();
  void on_doubleSpinBox_rf_freq_editingFinished();
  void exampleSlot();
  void on_actionStop_triggered();
  void on_checkBox_FileAsSource_clicked();
  void on_checkBox_enable_shortcut_clicked();
  void on_lineEdit_FileName_editingFinished();
  void on_actionSpectrum_changed();
  void on_actionDifference_changed();
  void on_actionUplink_changed();
  void on_actionDownlink_changed();
  void on_actionSave_Settings_changed();
  void on_actionplot1_changed();
  void on_Select_file_button_clicked();
  void on_lineEdit_FileName_textChanged(const QString &arg1);
  void on_actionUse_File_as_Source_changed();
  void on_actionTile_Windows_triggered();
  void on_actionDownlink_Plots_changed();
  void on_actionRNTI_Table_changed();
  void on_spinBox_Prb_valueChanged(int arg1);
  void on_slider_hist_threshold_valueChanged(int val);


  // Color
  //Color:
  void on_pushButton_uplink_color_clicked();
  void on_pushButton_downlink_color_clicked();
  void range_slider_value_changed(int value);

protected:
  void wheelEvent(QWheelEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *e) override;

private:

  PerformancePlot *perf_plot = nullptr;
  RNTITable *rnti_table = nullptr;
  Waterfall *dl_alloc = nullptr;
  Waterfall *ul_alloc = nullptr;
  Waterfall *diff_alloc = nullptr;
  Waterfall *dl_spec = nullptr;

  // Functions:

  void update_cell_config_fields();
  bool get_args_from_file(const QString filename);

  //  [SUBWINDOW]_start(bool)

  void handle_dl_alloc(bool start);
  void handle_ul_alloc(bool start);
  void handle_diff_alloc(bool start);
  void handle_dl_spec(bool start);
  void handle_perf_plot(bool start);
  void handle_rnti_table(bool start);

  // Color Menu:
  Colorpicker* cp;

  // Setting Class:
  Settings glob_settings;

  //Objects
  SpectrumAdapter spectrumAdapter;

  // Threads
  EyeThread eyeThread;
  std::shared_ptr<DCIGUIConsumer> guiConsumer;
  Args& eyeArgs;  // the actual object is part of settings

  //Subwindow Variables:
  QWidget *uplink_window = nullptr;
  QWidget *downlink_window = nullptr;
  QWidget *spectrum_window = nullptr;
  QWidget *diff_window = nullptr;

  QMdiSubWindow *uplink_subwindow = nullptr;
  QMdiSubWindow *downlink_subwindow = nullptr;
  QMdiSubWindow *spectrum_subwindow = nullptr;
  QMdiSubWindow *diff_subwindow = nullptr;

  bool active_eye   = false;
  //Files

  FILE *settings;

  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
