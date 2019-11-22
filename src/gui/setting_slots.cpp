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
#include <iostream>
#include <string>
#include <QDebug>
#include "file_input_output.h"
#include "falcon/definitions.h"
#include "falcon/meas/TrafficGenerator.h"
#include "falcon/meas/AuxModem.h"
#include "qcustomplot/qcustomplot.h"
#include "settings.h"

void MainWindow::on_doubleSpinBox_rf_freq_editingFinished() { //RF-Freq changed:
  glob_settings.glob_args.eyeArgs.rf_freq = ui->doubleSpinBox_rf_freq->value() * (1000 * 1000);   //Save Value to glob_args
  ui->lcdNumber_rf_freq->display(glob_settings.glob_args.eyeArgs.rf_freq / (1000 * 1000));  //Display rf_freq

  if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.
}

void MainWindow::exampleSlot() {
  on_doubleSpinBox_rf_freq_editingFinished();
}

void MainWindow::on_checkBox_FileAsSource_clicked() {
  glob_settings.glob_args.gui_args.use_file_as_source = ui->checkBox_FileAsSource->isChecked(); //Store checkbox Flag

  if(glob_settings.glob_args.gui_args.save_settings) {
    glob_settings.store_settings();  //If save_settings = true, save to file.
  }
}

void MainWindow::on_lineEdit_FileName_editingFinished() {
  glob_settings.glob_args.gui_args.path_to_file = ui->lineEdit_FileName->text().toLatin1().data(); //Store line as path to file.

  if(glob_settings.glob_args.gui_args.save_settings) {
    glob_settings.store_settings();  //If save_settings = true, save to file.
  }
}

void MainWindow::on_actionUse_File_as_Source_changed() {
  glob_settings.glob_args.gui_args.use_file_as_source = ui->actionUse_File_as_Source->isChecked(); //Store checkbox Flag

  if(glob_settings.glob_args.gui_args.save_settings) {
    glob_settings.store_settings();  //If save_settings = true, save to file.
  }
}

void MainWindow::on_actionSpectrum_changed() {
  glob_settings.glob_args.gui_args.show_spectrum = ui->actionSpectrum->isChecked();

  if(glob_settings.glob_args.gui_args.save_settings) {
    glob_settings.store_settings();  //If save_settings = true, save to file.
  }
  spectrum_start(glob_settings.glob_args.gui_args.show_spectrum);
}

void MainWindow::on_actionDifference_changed() {
  glob_settings.glob_args.gui_args.show_diff = ui->actionDifference->isChecked();

  if(glob_settings.glob_args.gui_args.save_settings) {
    glob_settings.store_settings();  //If save_settings = true, save to file.
  }
  diff_start(glob_settings.glob_args.gui_args.show_diff);
}

void MainWindow::on_actionUplink_changed() {
  glob_settings.glob_args.gui_args.show_uplink = ui->actionUplink->isChecked();

  if(glob_settings.glob_args.gui_args.save_settings) {
    glob_settings.store_settings();  //If save_settings = true, save to file.
  }
  uplink_start(glob_settings.glob_args.gui_args.show_uplink);
}

void MainWindow::on_actionDownlink_changed() {
  glob_settings.glob_args.gui_args.show_downlink = ui->actionDownlink->isChecked();

  if(glob_settings.glob_args.gui_args.save_settings) {
    glob_settings.store_settings();  //If save_settings = true, save to file.
  }
  downlink_start(glob_settings.glob_args.gui_args.show_downlink);
}

void MainWindow::on_actionSave_Settings_changed() {
  glob_settings.glob_args.gui_args.save_settings = ui->actionSave_Settings->isChecked();
  glob_settings.store_settings();  //save to file once.
}

void MainWindow::on_actionplot1_changed() {
  glob_settings.glob_args.gui_args.show_performance_plot = ui->actionplot1->isChecked();
  if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();
  performance_plots_start(glob_settings.glob_args.gui_args.show_performance_plot);
}

void MainWindow::on_actionDownlink_Plots_changed(){
  /* glob_settings.glob_args.gui_args.show_plot_downlink = ui->actionDownlink_Plots->isChecked();
  if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();*/
}
