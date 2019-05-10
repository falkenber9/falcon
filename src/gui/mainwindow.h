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

#include "qcustomplot/qcustomplot.h"

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

  void draw(float *inc, float *dec);
  void draw_ul(const ScanLineLegacy*);
  void draw_dl(const ScanLineLegacy*);
  void draw_spectrum(const ScanLineLegacy*);
  void draw_spectrum_diff(const ScanLineLegacy*);
  void draw_rnti_hist(const ScanLineLegacy*);
  void got_update();
  void spectrum_window_destroyed();
  void on_actionNew_triggered();
  void on_spinBox_rf_freq_editingFinished();
  void on_actionStop_triggered();
  //void on_checkBox_ShowSpectrum_clicked();
  //void on_checkBox_ShowDifference_clicked();
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

protected:
  void mousePressEvent(QMouseEvent *event) override;    // Klick and scroll per mousewheel
  void wheelEvent(QWheelEvent *event) override;         //

private slots:
  void on_actionUse_File_as_Source_changed();

private:

  //Functions:

  bool get_infos_from_file(QString filename, volatile prog_args_t& args);
  void add_data_to_plot(int data, QLineSeries *series, int *buffer);

  // QCustomPlots:
  void setupPlot(PlotsType_t plottype, QCustomPlot *plot);
  void addData(PlotsType_t plottype, QCustomPlot *plot, const ScanLineLegacy *data);
  QCustomPlot *testplot;
  //


  char buffer[50];  // Buffer für Textumwandlung
  int number = 0;

  //Setting Class:

  Settings glob_settings;

  // Setting Variables:

  int mouse_wheel_sens = 4;
  bool save_settings = true;
  double rf_freq = -1.0;
  bool use_file_as_source = true;

  //   Spectrogram:

  int spectrogram_line_count = 300;
  int spectrogram_line_shown = 150;
  int spectrogram_line_width = 50;

  // Threads
  ScanThread* scanThread;
  DecoderThread* decoderThread;

  //Objekte

  SpectrumAdapter spectrumAdapter;

  //Subwindow Variables:

  QSize windowsize_tmp_a;  // Windowsize for rescaling
  QSize windowsize_tmp_b;  // Windowsize for rescaling
  QSize windowsize_tmp_c;  // Windowsize for rescaling
  QSize windowsize_tmp_d;
  QSize windowsize_tmp_plot_a;

  QWidget *a_window;
  QWidget *b_window;
  QWidget *c_window;
  QWidget *d_window;

  QWidget *spectrum_view_window;

  bool spectrum_view_on = false;
  bool subwindow_state  = true;
  //bool show_spectrum    = false;
  //bool show_difference  = false;
  //bool show_downlink    = true;
  //bool show_uplink      = false;

  bool show_plot1 = false;

  //Charts:

  QWidget *plot_a_window;

  QChart *plot_a_chart;

  QLabel *plot_a_x_axis_1;
  QLabel *plot_a_x_axis_2;

  QChartView *chart_a_view;

  QLineSeries *plot_a_downlink;

  QPoint *point_xy = new QPoint;

  int *chart_a_buffer;

    //Chartposition and Size:
  int chart_a_size = SPECTROGRAM_LINE_SHOWN / 2;
  int chart_a_position = 0;
  int chart_a_port = SPECTROGRAM_LINE_SHOWN / 2;


  // *charts

  Spectrum *spectrum_view_ul;
  Spectrum *spectrum_view_dl;
  Spectrum *spectrum_view;
  Spectrum *spectrum_view_diff;

  bool spectrum_paused = false;

  //Files

  FILE *settings;

  Ui::MainWindow *ui;
};



#endif // MAINWINDOW_H
