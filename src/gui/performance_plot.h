#ifndef PERFORMANCE_PLOT_H
#define PERFORMANCE_PLOT_H

#pragma once

#include "qcustomplot/qcustomplot.h"
#include "settings.h"
#include "adapters_qt/SpectrumAdapter.h"
#include "plots.h"


class PerformancePlot : public QObject{
  Q_OBJECT
public:
  // Handle pointers on Main windows settings, spectrumAdapter and mdiArea
  explicit PerformancePlot(Settings* p_glob_settings, SpectrumAdapter *spectrumAdapter, QMdiArea *mdiArea);
  ~PerformancePlot();
  QMdiSubWindow* getSubwindow();
  QWidget* getWindow();

public slots:
  void activate();
  void deactivate();
  void update_plot_color();

private:
  QMdiArea* mdiArea = nullptr;
  Settings* glob_settings = nullptr;

  QGridLayout *gridLayout_a = nullptr;
  SpectrumAdapter* spectrumAdapter = nullptr;

  QCustomPlot *plot_mcs_idx = nullptr;
  QCustomPlot *plot_throughput = nullptr;
  QCustomPlot *plot_prb = nullptr;
  QCustomPlot *plot_rnti_hist = nullptr;
  QWidget *plot_a_window = nullptr;
  QMdiSubWindow *plot_a_subwindow = nullptr;
  QSlider *plot_mean_slider_a = nullptr;
  QLabel  *plot_mean_slider_label_a = nullptr;
  QLabel  *plot_mean_slider_label_b = nullptr;

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
  double throughput_upper = 0;
  double mcs_tbs_sum_uplink      = 0;
  double mcs_tbs_sum_downlink    = 0;
  QTimer throughput_rescale_timer;

  // Length of resource blocks
  double* l_prb = nullptr;
  double l_prb_sum_uplink        = 0;
  double l_prb_sum_downlink      = 0;

  void performance_plots_start(bool start);
  void setupPlot(PlotsType_t plottype, QCustomPlot *plot);
  void addData(PlotsType_t plottype, QCustomPlot *plot, const ScanLineLegacy *data);

private slots:
  void replot_perf();
  void draw_plot_uplink();
  void draw_plot_downlink();
  void draw_rnti_hist(const ScanLineLegacy *line);
  void calc_performance_data(const ScanLineLegacy*);
  void throughput_upper_limit_decay();

};

#endif // PERFORMANCE_PLOT_H
