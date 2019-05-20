#include "mainwindow.h"
#include "ui_mainwindow.h"

//   [SUBWINDOW]_start(bool) functions. true = start, false = stop

void MainWindow::downlink_start(bool start){
  if(spectrum_view_on){
    if(start){

      b_window = new QWidget();
      b_window->setObjectName("Downlink");
      b_window->setWindowTitle("Downlink RB Allocations");
      b_window->setGeometry(0,0,100,100);

      spectrum_view_dl = new Spectrum(b_window, &glob_settings);
      spectrum_view_dl->setObjectName("Spectrum View DL");
      spectrum_view_dl->setGeometry(0,0,100,100);

      b_subwindow = ui->mdiArea->addSubWindow(b_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      b_window->show();

      spectrum_view_dl->setFixedSize(b_window->size().width(),b_window->size().height());

      connect (b_window, SIGNAL(destroyed()),SLOT(downlink_destroyed()));
      connect (&spectrumAdapter, SIGNAL(update_dl(const ScanLineLegacy*)),SLOT(draw_dl(const ScanLineLegacy*)));
      connect (spectrum_view_dl, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
      spectrumAdapter.emit_downlink = true;

      //else disconnect (&spectrumAdapter, SIGNAL(update_dl(ScanLine*)),SLOT(draw_dl(ScanLine*)),&b_window);


    }else{

      //Deactivate Signals:
      disconnect(&spectrumAdapter, SIGNAL(update_dl(const ScanLineLegacy*)),this,SLOT(draw_dl(const ScanLineLegacy*)));
      spectrumAdapter.emit_downlink = false;
      b_window->disconnect();
      spectrum_view_dl->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(b_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(b_window);

      //delete pointers:
      // if(spectrum_view_dl != NULL)delete spectrum_view_dl;
      // if(b_window != NULL)delete b_window;

    }
    ui->mdiArea->tileSubWindows();
  }
}

void MainWindow::uplink_start(bool start){
  if(spectrum_view_on){
    if(start){

      a_window = new QWidget();
      a_window->setObjectName("Uplink");
      a_window->setWindowTitle("Uplink RB Allocations");
      a_window->setGeometry(0,0,100,100);

      spectrum_view_ul = new Spectrum(a_window, &glob_settings);
      spectrum_view_ul->setObjectName("Spectrum View UL");
      spectrum_view_ul->setGeometry(0,0,100,100);

      a_subwindow = ui->mdiArea->addSubWindow(a_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      a_window->show();

      spectrum_view_ul->setFixedSize(a_window->size().width(),a_window->size().height());

      connect (a_window, SIGNAL(destroyed()),SLOT(uplink_destroyed()));
      connect (&spectrumAdapter, SIGNAL(update_ul(const ScanLineLegacy*)),SLOT(draw_ul(const ScanLineLegacy*)));
      connect (spectrum_view_ul, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
      spectrumAdapter.emit_uplink = true;

    }else{

      //Deactivate Signals:
      disconnect(&spectrumAdapter, SIGNAL(update_ul(const ScanLineLegacy*)),this,SLOT(draw_ul(const ScanLineLegacy*)));
      spectrumAdapter.emit_uplink = false;
      a_window->disconnect();
      spectrum_view_ul->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(a_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(a_window);

    }
    ui->mdiArea->tileSubWindows();
  }

}

void MainWindow::spectrum_start(bool start){
  if(spectrum_view_on){
    if(start){

      c_window = new QWidget();
      c_window->setObjectName("Spectrum");
      c_window->setWindowTitle("Downlink Spectrum");
      c_window->setGeometry(0,0,100,100);

      spectrum_view = new Spectrum(c_window, &glob_settings);
      spectrum_view->setObjectName("Spectrum View");
      spectrum_view->setGeometry(0,0,100,100);

      c_subwindow = ui->mdiArea->addSubWindow(c_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      c_window->show();

      spectrum_view->setFixedSize(c_window->size().width(),c_window->size().height());

      connect (c_window, SIGNAL(destroyed()),SLOT(spectrum_destroyed()));
      connect (&spectrumAdapter, SIGNAL(update_spectrum(const ScanLineLegacy*)),SLOT(draw_spectrum(const ScanLineLegacy*)));
      connect (spectrum_view, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
      spectrumAdapter.emit_spectrum = true;

    }else{

      //Deactivate Signals:
      disconnect(&spectrumAdapter, SIGNAL(update_spectrum(const ScanLineLegacy*)),this,SLOT(draw_spectrum(const ScanLineLegacy*)));
      spectrumAdapter.emit_spectrum = false;
      c_window->disconnect();
      spectrum_view->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(c_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(c_window);

    }
    ui->mdiArea->tileSubWindows();
  }

}

void MainWindow::diff_start(bool start){

  if(spectrum_view_on){
    if(start){

      d_window = new QWidget();
      d_window->setObjectName("Spectrum Diff");
      d_window->setWindowTitle("Spectrum Difference");
      d_window->setGeometry(0,0,100,100);

      spectrum_view_diff = new Spectrum(d_window, &glob_settings);
      spectrum_view_diff->setObjectName("Spectrum View Diff");
      spectrum_view_diff->setGeometry(0,0,100,100);

      d_subwindow = ui->mdiArea->addSubWindow(d_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      d_window->show();

      spectrum_view_diff->setFixedSize(d_window->size().width(),d_window->size().height());

      connect (d_window, SIGNAL(destroyed()),SLOT(diff_destroyed()));
      connect (&spectrumAdapter, SIGNAL(update_spectrum_diff(const ScanLineLegacy*)),SLOT(draw_spectrum_diff(const ScanLineLegacy*)));
      connect (spectrum_view_diff, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
      spectrumAdapter.emit_difference = true;

    }else{

      //Deactivate Signals:
      disconnect(&spectrumAdapter, SIGNAL(update_spectrum_diff(const ScanLineLegacy*)),this,SLOT(draw_spectrum_diff(const ScanLineLegacy*)));
      spectrumAdapter.emit_difference = false;
      d_window->disconnect();
      spectrum_view_diff->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(d_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(d_window);

    }
    ui->mdiArea->tileSubWindows();
  }
}

void MainWindow::performance_plots_start(bool start){
  if(spectrum_view_on){
    if(start){
      //Generate Window PLOT_A_WINDOW:
      plot_a_window = new QWidget();
      plot_a_window->setObjectName("Plot a");
      plot_a_window->setWindowTitle("Uplink Plots");

      gridLayout_a = new QGridLayout(plot_a_window);              //Setup Gridlayout
      gridLayout_a->setSpacing(6);
      gridLayout_a->setContentsMargins(11, 11, 11, 11);
      gridLayout_a->setObjectName(QStringLiteral("gridLayout"));
      gridLayout_a->setSizeConstraint(QLayout::SetMaximumSize);
      gridLayout_a->setContentsMargins(0, 0, 0, 0);

      gridLayout_a->setColumnStretch(0,1);     //Set Stretchfactor 0 = no scaling, 1 = full scaling
      gridLayout_a->setColumnStretch(1,1);          
      gridLayout_a->setRowStretch(0,0);
      gridLayout_a->setRowStretch(1,1);
      gridLayout_a->setRowStretch(2,1);

      gridLayout_a->setRowMinimumHeight(0,20);  //Set minimum Size of grid-segment
      gridLayout_a->setRowMinimumHeight(1,100);
      gridLayout_a->setRowMinimumHeight(2,100);
      gridLayout_a->setColumnMinimumWidth(0,200);
      gridLayout_a->setColumnMinimumWidth(1,200);

      mcs_idx_plot_a = new QCustomPlot(plot_a_window);
      mcs_idx_plot_a->setObjectName(QStringLiteral("MCS_IDX Plot"));
      mcs_idx_plot_a->setGeometry(0,0,400,200);

      mcs_tbs_plot_a = new QCustomPlot(plot_a_window);
      mcs_tbs_plot_a->setObjectName(QStringLiteral("MCS_TBS Plot"));
      mcs_tbs_plot_a->setGeometry(0,200,400,200);

      prb_plot_a     = new QCustomPlot(plot_a_window);
      prb_plot_a    ->setObjectName(QStringLiteral("MCS_IDX Plot"));
      prb_plot_a    ->setGeometry(0,400,400,200);

      rnti_hist_plot_a = new QCustomPlot(plot_a_window);
      rnti_hist_plot_a->setObjectName(QStringLiteral("RNTI_HIST_PLOT"));
      rnti_hist_plot_a->setGeometry(0,400,400,200);

      plot_mean_slider_a = new QSlider(plot_a_window);
      plot_mean_slider_a->setGeometry(0, 600, 160, 20);
      plot_mean_slider_a->setMinimum(10);
      plot_mean_slider_a->setMaximum(500);
      plot_mean_slider_a->setValue(250);
      plot_mean_slider_a->setOrientation(Qt::Horizontal);

      plot_mean_slider_label_a = new QLabel(plot_a_window);
      plot_mean_slider_label_a->setGeometry(180, 600, 160, 20);
      plot_mean_slider_label_a->setNum(plot_mean_slider_a->value());

      plot_mean_slider_label_b = new QLabel(plot_a_window);
      plot_mean_slider_label_b->setGeometry(180, 600, 160, 20);
      plot_mean_slider_label_b->setText("           Average (ms)");

      gridLayout_a->addWidget(mcs_idx_plot_a            , 1, 0);  //Place Widgets into specific grid-segments: row, column
      gridLayout_a->addWidget(mcs_tbs_plot_a            , 1, 1);
      gridLayout_a->addWidget(prb_plot_a                , 2, 0);
      gridLayout_a->addWidget(rnti_hist_plot_a          , 2, 1);
      gridLayout_a->addWidget(plot_mean_slider_a        , 0, 0);
      gridLayout_a->addWidget(plot_mean_slider_label_a  , 0, 1);
      gridLayout_a->addWidget(plot_mean_slider_label_b  , 0, 1);

      connect (plot_mean_slider_a, SIGNAL(valueChanged(int)),plot_mean_slider_label_a,SLOT(setNum(int)));

      setupPlot(MCS_IDX_PLOT, mcs_idx_plot_a);
      setupPlot(MCS_TBS_PLOT, mcs_tbs_plot_a);
      setupPlot(PRB_PLOT    , prb_plot_a);
      setupPlot(RNTI_HIST   , rnti_hist_plot_a);

      //Add Subwindow to MDI Area

      plot_a_subwindow = ui->mdiArea->addSubWindow(plot_a_window,Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      plot_a_window->show();

      connect (&spectrumAdapter, SIGNAL(update_perf_plot_b(const ScanLineLegacy*)),SLOT(draw_plot(const ScanLineLegacy*)));
      connect (&spectrumAdapter, SIGNAL(update_perf_plot_a(const ScanLineLegacy*)),SLOT(draw_plot(const ScanLineLegacy*)));
      connect (&spectrumAdapter, SIGNAL(update_rnti_hist(const ScanLineLegacy*)),SLOT(draw_rnti_hist(const ScanLineLegacy*)));
      spectrumAdapter.emit_perf_plot_a = true;
      spectrumAdapter.emit_perf_plot_b = true;
      spectrumAdapter.emit_rnti_hist   = true;

    }else{

      //Deactivate Signals:

      disconnect (&spectrumAdapter, SIGNAL(update_perf_plot_a(const ScanLineLegacy*)),this,SLOT(draw_plot(const ScanLineLegacy*)));
      disconnect (&spectrumAdapter, SIGNAL(update_perf_plot_b(const ScanLineLegacy*)),this,SLOT(draw_plot(const ScanLineLegacy*)));
      disconnect (&spectrumAdapter, SIGNAL(update_rnti_hist(const ScanLineLegacy*)),this,SLOT(draw_rnti_hist(const ScanLineLegacy*)));

      spectrumAdapter.emit_perf_plot_a = false;
      spectrumAdapter.emit_perf_plot_b = false;
      spectrumAdapter.emit_rnti_hist   = false;

      plot_a_window->disconnect();
      plot_mean_slider_a->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(plot_a_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(plot_a_window);

    }
    ui->mdiArea->tileSubWindows();
  }
}

// Color selection menu for dynamic colors of plots and spectrum:

void MainWindow::setup_color_menu(){

  glob_settings.glob_args.gui_args.downlink_plot_color = QColor(40,110,255); //Blue
  glob_settings.glob_args.gui_args.uplink_plot_color   = QColor(255,110,40); //Orange

  downlink_palette.setColor(QPalette::Window, glob_settings.glob_args.gui_args.downlink_plot_color);
  uplink_palette.  setColor(QPalette::Window, glob_settings.glob_args.gui_args.uplink_plot_color);

  ui->color_label_downlink->setAutoFillBackground(true);
  ui->color_label_downlink->setPalette(downlink_palette);
  ui->color_label_uplink->setAutoFillBackground(true);
  ui->color_label_uplink->setPalette(uplink_palette);
  //ui->color_label->setText("What ever text");

  color_dialog = new QColorDialog(ui->color_settings);
  color_dialog->setObjectName("CD");
  color_dialog->setWindowTitle("Color Dialog");
  color_dialog->setGeometry(0,0,100,100);

  connect(color_dialog,SIGNAL(currentColorChanged(const QColor)),SLOT(set_color(const QColor)));



  color_range_slider = new RangeWidget(Qt::Horizontal,ui->color_settings);
  color_range_slider->setObjectName(QStringLiteral("horizontalSlider"));
  color_range_slider->setGeometry(QRect(30, 130, 160, 20));
  // color_range_slider->setOrientation(Qt::Horizontal);
  color_range_slider->setRange(0,50000);
  color_range_slider->setFirstValue(0);
  color_range_slider->setSecondValue(50000);
  connect(color_range_slider,SIGNAL(secondValueChanged(int)),SLOT(range_slider_value_changed(int)));
  connect(color_range_slider,SIGNAL(firstValueChanged(int)),SLOT(range_slider_value_changed(int)));

}

void MainWindow::set_color(const QColor &color){

  // Change color of display Label:
  if(downlink_color_active){
    glob_settings.glob_args.gui_args.downlink_plot_color = color;
    downlink_palette.setColor(QPalette::Window, glob_settings.glob_args.gui_args.downlink_plot_color);
    ui->color_label_downlink->setPalette(downlink_palette);
  }else{
    glob_settings.glob_args.gui_args.uplink_plot_color = color;
    uplink_palette.setColor(QPalette::Window, glob_settings.glob_args.gui_args.uplink_plot_color);
    ui->color_label_uplink->setPalette(uplink_palette);
  }

  if(spectrum_view_on) update_plot_color();

}

void MainWindow::on_pushButton_downlink_color_clicked()
{
  downlink_color_active = true;
  color_dialog->show();
}

void MainWindow::on_pushButton_uplink_color_clicked()
{
  downlink_color_active = false;
  color_dialog->show();
}

void MainWindow::range_slider_value_changed(int value){
  if(color_range_slider->firstValue() > color_range_slider->secondValue()){
    //qDebug()<< "Min: " << color_range_slider->secondValue() << " Max: "<< color_range_slider->firstValue() ;
   spectrum_view->max_intensity = color_range_slider->firstValue();
   spectrum_view->min_intensity = color_range_slider->secondValue();
  }else{
    //qDebug()<< "Min: " << color_range_slider->firstValue() << " Max: "<< color_range_slider->secondValue() ;
    spectrum_view->max_intensity = color_range_slider->secondValue();
    spectrum_view->min_intensity = color_range_slider->firstValue();
  }
  spectrum_view->intensity_factor = (1.125 * (float)USHRT_MAX) / (spectrum_view->max_intensity - spectrum_view->min_intensity);   // Calculate Intensity factor for dynamic spectrum
}
