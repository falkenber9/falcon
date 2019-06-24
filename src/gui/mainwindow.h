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

#include <QMainWindow>
#include "spectrum.h"
#include "falcon/CCSnifferInterfaces.h"
#include "adapters_qt/SpectrumAdapter.h"
#include "model_dummy/ScanThread.h"
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
  void draw_plot(const ScanLineLegacy*);
  //void draw_plot_a(const ScanLineLegacy*);
  //void draw_plot_b(const ScanLineLegacy*);
  void draw_rnti_hist(const ScanLineLegacy *line);
  //void draw_rnti_hist_b(const ScanLineLegacy *line);
  //void spectrum_window_destroyed();
  void on_actionNew_triggered();
  void on_spinBox_rf_freq_editingFinished();
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

  bool get_infos_from_file(QString filename, volatile prog_args_t& args);

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

  QCustomPlot *mcs_idx_plot_a;
  QCustomPlot *mcs_tbs_plot_a;
  QCustomPlot *prb_plot_a;
  QCustomPlot *rnti_hist_plot_a;
  QWidget *plot_a_window;
  QMdiSubWindow *plot_a_subwindow = NULL;
  QSlider *plot_mean_slider_a;
  QLabel  *plot_mean_slider_label_a;
  QLabel  *plot_mean_slider_label_b;


  //Variables for plots:

  uint32_t sfn_old_a        = 0;
  uint32_t sfn_old_b        = 0;
  uint32_t mcs_idx_sum_a    = 0;
  uint32_t mcs_idx_sum_b    = 0;
  int mcs_idx_sum_counter_a = 0;
  int mcs_idx_sum_counter_b = 0;
  uint32_t mcs_tbs_sum_a    = 0;
  uint32_t mcs_tbs_sum_b    = 0;
  uint32_t l_prb_sum_a      = 0;
  uint32_t l_prb_sum_b      = 0;
  int plot_counter_a        = 0;
  int plot_counter_b        = 0;

  uint32_t mcs_idx_sum_sum_a    = 0;
  uint32_t mcs_idx_sum_sum_b    = 0;
  uint32_t mcs_tbs_sum_sum_a    = 0;
  uint32_t mcs_tbs_sum_sum_b    = 0;
  uint32_t l_prb_sum_sum_a      = 0;
  uint32_t l_prb_sum_sum_b      = 0;
  uint32_t sum_sum_counter_a    = 1;
  uint32_t sum_sum_counter_b    = 1;


  // Setting Class:

  Settings glob_settings;

  // Setting Variables:

  int mouse_wheel_sens = 4;
  bool save_settings = true;
  double rf_freq = -1.0;
  bool use_file_as_source = true;

  // Spectrogram:

  Spectrum *spectrum_view_ul   = NULL;
  Spectrum *spectrum_view_dl   = NULL;
  Spectrum *spectrum_view      = NULL;
  Spectrum *spectrum_view_diff = NULL;

  bool spectrum_paused       = false;
  int spectrogram_line_count = 300;
  int spectrogram_line_shown = 150;
  int spectrogram_line_width = 50;




  // Threads
  ScanThread* scanThread;
  DecoderThread* decoderThread;

  //Objects

  SpectrumAdapter spectrumAdapter;

  //Subwindow Variables:

  QSize windowsize_tmp_a;  // Windowsize for rescaling
  QSize windowsize_tmp_b;  // Windowsize for rescaling
  QSize windowsize_tmp_c;  // Windowsize for rescaling
  QSize windowsize_tmp_d;
  QSize windowsize_tmp_plot_a;

  QWidget *a_window = NULL;
  QWidget *b_window = NULL;
  QWidget *c_window = NULL;
  QWidget *d_window = NULL;

  QMdiSubWindow *a_subwindow = NULL;
  QMdiSubWindow *b_subwindow = NULL;
  QMdiSubWindow *c_subwindow = NULL;
  QMdiSubWindow *d_subwindow = NULL;

  QWidget *spectrum_view_window;

  bool spectrum_view_on   = false;
  bool subwindow_state    = true;


  //Files

  FILE *settings;

  Ui::MainWindow *ui;
};



#endif // MAINWINDOW_H
