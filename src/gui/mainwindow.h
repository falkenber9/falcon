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
//#include "model_dummy/ScanThread.h"

#include "model/EyeThread.h"
#include "model_dummy/cni_cc_decoderThread.h"

#include "QTextBrowser"
#include "stdio.h"
#include "QtCharts"
#include "QtCharts/QLineSeries"
#include "settings.h"
#include "plots.h"
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

  void draw_ul(const ScanLineLegacy*);
  void draw_dl(const ScanLineLegacy*);
  void draw_spectrum(const ScanLineLegacy*);
  void draw_spectrum_diff(const ScanLineLegacy*);
  void calc_performance_data(const ScanLineLegacy*);
  void replot_perf();
  void draw_plot_uplink();
  void draw_plot_downlink();
  //void draw_plot_a(const ScanLineLegacy*);
  //void draw_plot_b(const ScanLineLegacy*);
  void draw_rnti_hist(const ScanLineLegacy *line);
  //void draw_rnti_hist_b(const ScanLineLegacy *line);
  //void spectrum_window_destroyed();
  void on_actionStart_triggered();
  void on_doubleSpinBox_rf_freq_editingFinished();
  void exampleSlot();
  void on_actionStop_triggered();
  void on_checkBox_FileAsSource_clicked();
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
  void on_pushButton_downlink_color_clicked();
  void SubWindow_mousePressEvent();
  void on_spinBox_Prb_valueChanged(int arg1);

  //Color:
  void on_pushButton_uplink_color_clicked();
  void set_color(const QColor &color);
  void range_slider_value_changed(int value);

protected:
  //void mousePressEvent(QMouseEvent *event) override;    // Klick and scroll per mousewheel
  void wheelEvent(QWheelEvent *event) override;         //

  void dropEvent(QDropEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *e) override;

private:

  // Functions:

  void update_cell_config_fields();
  bool get_args_from_file(const QString filename);

  //  [SUBWINDOW]_start(bool)

  void downlink_start(bool start);
  void uplink_start(bool start);
  void diff_start(bool start);
  void spectrum_start(bool start);
  void performance_plots_start(bool start);

  // Color Menu:
  void setup_color_menu();

  bool downlink_color_active;
  QColorDialog *color_dialog;
  RangeWidget *color_range_slider;
  QPalette downlink_palette;
  QPalette uplink_palette;


  // QCustomPlots:

  QGridLayout *gridLayout_a;
  //  QGridLayout *gridLayout_b;

  void setupPlot(PlotsType_t plottype, QCustomPlot *plot);
  void addData(PlotsType_t plottype, QCustomPlot *plot, const ScanLineLegacy *data);
  void update_plot_color();

  QCustomPlot *plot_mcs_idx;
  QCustomPlot *plot_throughput;
  QCustomPlot *plot_prb;
  QCustomPlot *plot_rnti_hist;
  QWidget *plot_a_window;
  QMdiSubWindow *plot_a_subwindow = NULL;
  QSlider *plot_mean_slider_a;
  QLabel  *plot_mean_slider_label_a;
  QLabel  *plot_mean_slider_label_b;

  QTimer fps_timer;
  QTimer avg_timer_uplink;
  QTimer avg_timer_downlink;

  // QCP graphs
  QCPGraph* graph_current = nullptr;
  QCPGraph* graph_mcs_idx = nullptr;
  QCPGraph* graph_throughput   =  nullptr;
  QCPGraph* graph_prb = nullptr;

  //Variables for plots:
  std::vector<double> rnti_x_axis;
  xAxisTicks xAT;
  QSharedPointer<QCPAxisTickerText> xTicker;

  /*  Pointer to counter pairs:
     *  T* ptr;
     *  T  cnt_uplink;
     *  T  cnt_downlink;     *
     */
  // Mutex
  std::mutex perf_mutex;

  // Timestamps
  double* last_timestamp = nullptr;
  double last_timestamp_uplink = 0;
  double last_timestamp_downlink = 0;

  // Subframe index
  uint32_t* sf_idx_old = nullptr;
  uint32_t sf_idx_old_uplink        = 0;
  uint32_t sf_idx_old_downlink      = 0;

  // Number of received subframes
  double* nof_received_sf = nullptr;
  double nof_received_sf_uplink    = 0;
  double nof_received_sf_downlink    = 0;

  // Modulation and coding scheme index
  double* mcs_idx = nullptr;
  double mcs_idx_uplink    = 0;
  double mcs_idx_sum_downlink    = 0;

  // Number of allocations
  int* nof_allocations = nullptr;
  int nof_allocations_uplink = 0;
  int nof_allocations_downlink = 0;

  // Transportblocksize
  double* mcs_tbs = nullptr;
  double mcs_tbs_sum_uplink    = 0;
  double mcs_tbs_sum_downlink    = 0;

  // Length of resource blocks
  double* l_prb = nullptr;
  double l_prb_sum_uplink      = 0;
  double l_prb_sum_downlink      = 0;


  // Setting Class:
  Settings glob_settings;

  // Spectrogram:
  Spectrum *spectrum_view_ul   = nullptr;
  Spectrum *spectrum_view_dl   = nullptr;
  Spectrum *spectrum_view      = nullptr;
  Spectrum *spectrum_view_diff = nullptr;

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

  bool spectrum_view_on   = false;
  //Files

  FILE *settings;

  Ui::MainWindow *ui;
};



#endif // MAINWINDOW_H
