#ifndef PERFORMANCE_PLOT_H
#define PERFORMANCE_PLOT_H

#pragma once

#include "qcustomplot/qcustomplot.h"
#include "settings.h"
#include "adapters_qt/SpectrumAdapter.h"
#include "plots.h"

struct CellMetrics {

  // TTI
  bool fresh = true;
  uint32_t tti_start = 0;
  uint32_t tti_current = 0;

  // Number of received subframes
  double nof_tti_with_dci = 0;

  // Modulation and coding scheme index
  double mcs_idx = 0;

  // Number of allocations
  uint32_t nof_dci = 0;

  // Transportblocksize
  double mcs_tbs = 0;

  // Number of PRBs
  double l_prb = 0;
};


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
  void setFPS(int fps);

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

  double throughput_max = 0;
  QTimer throughput_rescale_timer;

  CellMetrics metrics_ul;
  CellMetrics metrics_dl;

  void performance_plots_start(bool start);
  void setupPlot(PlotsType_t plottype, QCustomPlot *plot);
  void addData(PlotsType_t plottype, QCustomPlot *plot, const ScanLineLegacy *data);

  void draw_plot(LINKTYPE direction, CellMetrics& metrics);

private slots:
  void replot_perf();
  void draw_plot_uplink();
  void draw_plot_downlink();
  void draw_rnti_hist(const ScanLineLegacy *line);
  void calc_performance_data(const ScanLineLegacy*);
  void throughput_upper_limit_decay();

};

#endif // PERFORMANCE_PLOT_H
