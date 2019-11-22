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
#include "mainwindow.h"
#include "ui_mainwindow.h"

//   [SUBWINDOW]_start(bool) functions. true = start, false = stop
#define FPS_TIME_INTERVAL_MS 16

void MainWindow::downlink_start(bool start){
  if(spectrum_view_on){
    if(start){

      // Initialize objects
      downlink_window = new QWidget();
      spectrum_view_dl = new Spectrum(downlink_window, &glob_settings);

      // Register widget in subwindow
      downlink_subwindow = ui->mdiArea->addSubWindow(downlink_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);

      // Set properties
      downlink_window->setObjectName("Downlink");
      downlink_window->setWindowTitle("Downlink RB Allocations");

      spectrum_view_dl->setObjectName("Spectrum View DL");

      // Maximize widget and use size to resize spectrum view
      downlink_subwindow->showMaximized();
      spectrum_view_dl->setFixedSize(downlink_window->size().width(),downlink_window->size().height());

      // Connect signals/slots
      connect (downlink_window, SIGNAL(destroyed()),SLOT(downlink_destroyed()));
      connect (&spectrumAdapter, SIGNAL(update_dl(const ScanLineLegacy*)),SLOT(draw_dl(const ScanLineLegacy*)));
      connect (spectrum_view_dl, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
      spectrumAdapter.emit_downlink = true;

      // Finally show widget
      downlink_window->show();

    }else{

      //Deactivate Signals:
      disconnect(&spectrumAdapter, SIGNAL(update_dl(const ScanLineLegacy*)),this,SLOT(draw_dl(const ScanLineLegacy*)));
      spectrumAdapter.emit_downlink = false;
      downlink_window->disconnect();
      spectrum_view_dl->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(downlink_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(downlink_window);

      //delete pointers:
      if(spectrum_view_dl != nullptr)delete spectrum_view_dl;
      // if(b_window != NULL)delete b_window;

    }
    ui->mdiArea->tileSubWindows();
  }
}

void MainWindow::uplink_start(bool start){
  if(spectrum_view_on){
    if(start){

      // Initialize objects
      uplink_window = new QWidget();
      spectrum_view_ul = new Spectrum(uplink_window, &glob_settings);

      // Register widget in subwindow
      uplink_subwindow = ui->mdiArea->addSubWindow(uplink_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);

      // Set properties
      uplink_window->setObjectName("Uplink");
      uplink_window->setWindowTitle("Uplink RB Allocations");

      spectrum_view_ul->setObjectName("Spectrum View UL");

      // Maximize widget and use size to resize spectrum view
      uplink_subwindow->showMaximized();
      spectrum_view_ul->setFixedSize(uplink_window->size().width(),uplink_window->size().height());

      // Connect signals/slots
      connect (uplink_window, SIGNAL(destroyed()),SLOT(uplink_destroyed()));
      connect (&spectrumAdapter, SIGNAL(update_ul(const ScanLineLegacy*)),SLOT(draw_ul(const ScanLineLegacy*)));
      connect (spectrum_view_ul, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
      spectrumAdapter.emit_uplink = true;

      // Finally show widget
      uplink_window->show();

    }else{

      //Deactivate Signals:
      disconnect(&spectrumAdapter, SIGNAL(update_ul(const ScanLineLegacy*)),this,SLOT(draw_ul(const ScanLineLegacy*)));
      spectrumAdapter.emit_uplink = false;
      uplink_window->disconnect();
      spectrum_view_ul->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(uplink_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(uplink_window);

      //delete pointers:
      if(spectrum_view_ul != nullptr)delete spectrum_view_ul;

    }
    ui->mdiArea->tileSubWindows();
  }

}

void MainWindow::spectrum_start(bool start){
  if(spectrum_view_on){
    if(start){

      // Initialize objects
      spectrum_window = new QWidget();
      spectrum_view = new Spectrum(spectrum_window, &glob_settings);

      // Register widget in subwindow
      spectrum_subwindow = ui->mdiArea->addSubWindow(spectrum_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);

      // Set properties
      spectrum_window->setObjectName("Spectrum");
      spectrum_window->setWindowTitle("Downlink Spectrum");

      spectrum_view->setObjectName("Spectrum View");

      // Maximize widget and use size to resize spectrum view
      spectrum_subwindow->showMaximized();
      spectrum_view->setFixedSize(spectrum_window->size().width(),spectrum_window->size().height());

      // Connect signals/slots
      connect (spectrum_window, SIGNAL(destroyed()),SLOT(spectrum_destroyed()));
      connect (&spectrumAdapter, SIGNAL(update_spectrum(const ScanLineLegacy*)),SLOT(draw_spectrum(const ScanLineLegacy*)));
      connect (spectrum_view, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
      spectrumAdapter.emit_spectrum = true;

      // Finally show widget
      spectrum_window->show();

    }else{

      //Deactivate Signals:
      disconnect(&spectrumAdapter, SIGNAL(update_spectrum(const ScanLineLegacy*)),this,SLOT(draw_spectrum(const ScanLineLegacy*)));
      spectrumAdapter.emit_spectrum = false;
      spectrum_window->disconnect();
      spectrum_view->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(spectrum_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(spectrum_window);

      //delete pointers:
      if(spectrum_view != nullptr)delete spectrum_view;

    }
    ui->mdiArea->tileSubWindows();
  }

}

void MainWindow::diff_start(bool start){

  if(spectrum_view_on){
    if(start){

      // Initialize objects
      diff_window = new QWidget();
      spectrum_view_diff = new Spectrum(diff_window, &glob_settings);

      // Register widget in subwindow
      diff_subwindow = ui->mdiArea->addSubWindow(diff_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);

      // Set properties
      diff_window->setObjectName("Spectrum Diff");
      diff_window->setWindowTitle("Spectrum Difference");

      spectrum_view_diff->setObjectName("Spectrum View Diff");

      // Maximize widget and use size to resize spectrum view
      diff_subwindow->showMaximized();
      spectrum_view_diff->setFixedSize(diff_window->size().width(),diff_window->size().height());

      // Connect signals/slots
      connect (diff_window, SIGNAL(destroyed()),SLOT(diff_destroyed()));
      connect (&spectrumAdapter, SIGNAL(update_spectrum_diff(const ScanLineLegacy*)),SLOT(draw_spectrum_diff(const ScanLineLegacy*)));
      connect (spectrum_view_diff, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
      spectrumAdapter.emit_difference = true;

      // Finally show widget
      diff_window->show();

    }else{

      //Deactivate Signals:
      disconnect(&spectrumAdapter, SIGNAL(update_spectrum_diff(const ScanLineLegacy*)),this,SLOT(draw_spectrum_diff(const ScanLineLegacy*)));
      spectrumAdapter.emit_difference = false;
      diff_window->disconnect();
      spectrum_view_diff->disconnect();

      //Close Subwindow:
      ui->mdiArea->setActiveSubWindow(diff_subwindow);
      ui->mdiArea->closeActiveSubWindow();
      ui->mdiArea->removeSubWindow(diff_window);

      //delete pointers:
      if(spectrum_view_diff != nullptr)delete spectrum_view_diff;

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
      plot_a_window->setWindowTitle("Cell Activity");

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

      plot_mcs_idx = new QCustomPlot(plot_a_window);
      plot_mcs_idx->setObjectName(QStringLiteral("MCS_IDX Plot"));
      plot_mcs_idx->setGeometry(0,0,400,200);

      plot_throughput = new QCustomPlot(plot_a_window);
      plot_throughput->setObjectName(QStringLiteral("MCS_TBS Plot"));
      plot_throughput->setGeometry(0,200,400,200);

      plot_prb     = new QCustomPlot(plot_a_window);
      plot_prb    ->setObjectName(QStringLiteral("MCS_IDX Plot"));
      plot_prb    ->setGeometry(0,400,400,200);

      plot_rnti_hist = new QCustomPlot(plot_a_window);
      plot_rnti_hist->setObjectName(QStringLiteral("RNTI_HIST_PLOT"));
      plot_rnti_hist->setGeometry(0,400,400,200);

      plot_mean_slider_a = new QSlider(plot_a_window);
      plot_mean_slider_a->setGeometry(0, 600, 160, 20);
      plot_mean_slider_a->setMinimum(50);
      plot_mean_slider_a->setMaximum(500);
      plot_mean_slider_a->setValue(250);
      plot_mean_slider_a->setOrientation(Qt::Horizontal);

      plot_mean_slider_label_a = new QLabel(plot_a_window);
      plot_mean_slider_label_a->setGeometry(180, 600, 160, 20);
      plot_mean_slider_label_a->setNum(plot_mean_slider_a->value());

      plot_mean_slider_label_b = new QLabel(plot_a_window);
      plot_mean_slider_label_b->setGeometry(180, 600, 160, 20);
      plot_mean_slider_label_b->setText("           Average (ms)");


      gridLayout_a->addWidget(plot_mcs_idx            , 1, 0);  //Place Widgets into specific grid-segments: row, column
      gridLayout_a->addWidget(plot_throughput            , 1, 1);
      gridLayout_a->addWidget(plot_prb                , 2, 0);
      gridLayout_a->addWidget(plot_rnti_hist          , 2, 1);
      gridLayout_a->addWidget(plot_mean_slider_a        , 0, 0);
      gridLayout_a->addWidget(plot_mean_slider_label_a  , 0, 1);
      gridLayout_a->addWidget(plot_mean_slider_label_b  , 0, 1);

      connect (plot_mean_slider_a, SIGNAL(valueChanged(int)),plot_mean_slider_label_a,SLOT(setNum(int)));
      connect (&fps_timer, SIGNAL(timeout()), this, SLOT(replot_perf()));
      fps_timer.start(FPS_TIME_INTERVAL_MS);

      connect (&avg_timer_uplink, SIGNAL(timeout()), this, SLOT(draw_plot_uplink()));
      avg_timer_uplink.start(plot_mean_slider_a->value());
      connect(plot_mean_slider_a, SIGNAL(valueChanged(int)), &avg_timer_uplink, SLOT(start(int)));

      connect (&avg_timer_downlink, SIGNAL(timeout()), this, SLOT(draw_plot_downlink()));
      avg_timer_downlink.start(plot_mean_slider_a->value());
      connect(plot_mean_slider_a, SIGNAL(valueChanged(int)), &avg_timer_downlink, SLOT(start(int)));


      setupPlot(MCS_IDX_PLOT, plot_mcs_idx);
      setupPlot(MCS_TBS_PLOT, plot_throughput);
      setupPlot(PRB_PLOT    , plot_prb);
      setupPlot(RNTI_HIST   , plot_rnti_hist);

      //Add Subwindow to MDI Area

      plot_a_subwindow = ui->mdiArea->addSubWindow(plot_a_window,Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      plot_a_window->show();

      connect (&spectrumAdapter, SIGNAL(update_perf_plot_b(const ScanLineLegacy*)),SLOT(calc_performance_data(const ScanLineLegacy*)));
      connect (&spectrumAdapter, SIGNAL(update_perf_plot_a(const ScanLineLegacy*)),SLOT(calc_performance_data(const ScanLineLegacy*)));
      connect (&spectrumAdapter, SIGNAL(update_rnti_hist(const ScanLineLegacy*)),SLOT(draw_rnti_hist(const ScanLineLegacy*)));
      spectrumAdapter.emit_perf_plot_a = true;
      spectrumAdapter.emit_perf_plot_b = true;
      spectrumAdapter.emit_rnti_hist   = true;

    }else{

      //Deactivate Signals:
      disconnect (&spectrumAdapter, SIGNAL(update_perf_plot_a(const ScanLineLegacy*)),this,SLOT(calc_performance_data(const ScanLineLegacy*)));
      disconnect (&spectrumAdapter, SIGNAL(update_perf_plot_b(const ScanLineLegacy*)),this,SLOT(calc_performance_data(const ScanLineLegacy*)));
      disconnect (&spectrumAdapter, SIGNAL(update_rnti_hist(const ScanLineLegacy*)),this,SLOT(draw_rnti_hist(const ScanLineLegacy*)));

      spectrumAdapter.emit_perf_plot_a = false;
      spectrumAdapter.emit_perf_plot_b = false;
      spectrumAdapter.emit_rnti_hist   = false;

      // Stop timer activities
      fps_timer.stop();
      avg_timer_uplink.stop();
      avg_timer_downlink.stop();

      fps_timer.disconnect();
      avg_timer_uplink.disconnect();
      avg_timer_downlink.disconnect();

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
