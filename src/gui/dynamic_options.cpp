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

void MainWindow::handle_dl_alloc(bool nexist){
  if(nexist){
    //  If dl allocation object does not exist (by flag), create a new one and activate it instantly if eye is active
    //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
    if(dl_alloc != nullptr){delete dl_alloc; dl_alloc = nullptr;}
    dl_alloc = new Waterfall_DL(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ dl_alloc->activate(); }
  }else{
    if (dl_alloc != nullptr){
      delete dl_alloc;
      dl_alloc = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_ul_alloc(bool nexist){
  if(nexist){
      //  If ul allocation object does not exist (by flag), create a new one and activate it instantly if eye is active
      //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
      if(ul_alloc != nullptr){delete ul_alloc; ul_alloc = nullptr;}
      ul_alloc = new Waterfall_UL(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ ul_alloc->activate(); }
  }else{
    if (ul_alloc != nullptr){
      delete ul_alloc;
      ul_alloc = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_diff_alloc(bool nexist){
  if(nexist){
    //  If diff object does not exist (by flag), create a new one and activate it instantly if eye is active
    //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
    if(diff_alloc != nullptr){delete diff_alloc; diff_alloc = nullptr;}
    diff_alloc = new Waterfall_DIFF(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ diff_alloc->activate(); }
  }else{
    if (diff_alloc != nullptr){
      delete diff_alloc;
      diff_alloc = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_dl_spec(bool nexist){
  if(nexist){
      //  If spectrum object does not exist (by flag), create a new one and activate it instantly if eye is active
      //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
      if(dl_spec != nullptr){delete dl_spec; dl_spec = nullptr;}
      dl_spec = new Waterfall_SPEC(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ dl_spec->activate(); }
  }else{
    if (dl_spec != nullptr){
      delete dl_spec;
      dl_spec = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_perf_plot(bool nexist){
  if(nexist){
      //  If perf plot object does not exist (by flag), create a new one and activate it instantly if eye is active
      //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
      if(perf_plot != nullptr){delete perf_plot; perf_plot = nullptr;}
      perf_plot = new PerformancePlot(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ perf_plot->activate(); }
  }else{
    if (perf_plot != nullptr){
      delete perf_plot;
      perf_plot = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
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

  if(active_eye) perf_plot->update_plot_color();

}

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
