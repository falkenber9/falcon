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
#include "plots.h"

#include "qcustomplot/qcustomplot.h"

#include "settings.h"

#define CNI_GUI

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  eyeThread(),
  guiConsumer(new DCIGUIConsumer()),
  eyeArgs(glob_settings.glob_args.eyeArgs),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  cp = new Colorpicker(&glob_settings, ui);    //For Color Menu

  setAccessibleName(QString("FALCON GUI"));
  ui->mdiArea->tileSubWindows();

  //  ui->mdiArea->setActivationOrder(QMdiArea::CreationOrder); // Former default value
  ui->mdiArea->setActivationOrder(QMdiArea::ActivationHistoryOrder);

  //if store settings true: load settings
  if(glob_settings.glob_args.gui_args.save_settings)glob_settings.load_settings();

  // Set file_wrap flag to true for looping file data
  glob_settings.glob_args.eyeArgs.file_wrap = true;

  eyeThread.init();
  // Connect objects safely --> to be improved!
  guiConsumer->setThread(eyeThread);
  eyeThread.attachConsumer(guiConsumer);

  eyeThread.subscribe(&spectrumAdapter);

  //Settings are initialised on startup in constructor of settings class.
  //Init Checkboxes:
  ui->actionDifference->          setChecked(glob_settings.glob_args.gui_args.show_diff);
  ui->actionDownlink->            setChecked(glob_settings.glob_args.gui_args.show_downlink);
  ui->actionSpectrum->            setChecked(glob_settings.glob_args.gui_args.show_spectrum);
  ui->actionUplink->              setChecked(glob_settings.glob_args.gui_args.show_uplink);
  ui->actionplot1->               setChecked(glob_settings.glob_args.gui_args.show_performance_plot);
  ui->actionRNTI_Table->          setChecked(glob_settings.glob_args.gui_args.show_rnti);

  //ui->actionDownlink_Plots->      setChecked(glob_settings.glob_args.gui_args.show_plot_downlink);

  ui->lineEdit_FileName->         setText(glob_settings.glob_args.gui_args.path_to_file);
  ui->actionSave_Settings->       setChecked(glob_settings.glob_args.gui_args.save_settings);
  ui->actionUse_File_as_Source->  setChecked(glob_settings.glob_args.gui_args.use_file_as_source);
  ui->checkBox_FileAsSource ->    setChecked(glob_settings.glob_args.gui_args.use_file_as_source);
  if(ui->checkBox_FileAsSource->isChecked()) {
    QString filename = ui->lineEdit_FileName->text();
    if(!get_args_from_file(filename)) {
      qDebug() << "Could not load parameters from file source" << endl;
      return;
    }
  }

  // Set the timer sliders here as the values are needed on setup
  ui->slider_perf_fps->setValue(glob_settings.glob_args.gui_args.perf_fps);
  ui->slider_wf_fps->setValue(glob_settings.glob_args.gui_args.wf_fps);
  ui->spinBox_nof_sf_workers->setValue(glob_settings.glob_args.eyeArgs.nof_subframe_workers);
  ui->slider_hist_threshold->setValue(glob_settings.glob_args.eyeArgs.rnti_histogram_threshold);
  ui->slider_mouse_sensivity->setValue(glob_settings.glob_args.spectrum_args.mouse_wheel_sens);
  ui->slider_scrollback_buffer->setValue(glob_settings.glob_args.spectrum_args.spectrum_line_count);
  ui->slider_viewport->setValue(glob_settings.glob_args.spectrum_args.spectrum_line_shown);

  // After setting checkboxes, call on_ActionX_changed slots manually to ensure that the object get created (if necessary) even if the checkbox didn't change
  on_actionDownlink_changed();
  on_actionUplink_changed();
  on_actionSpectrum_changed();
  on_actionDifference_changed();
  on_actionRNTI_Table_changed();
  on_actionplot1_changed();

  //Init Path to File:
  setAcceptDrops(true);  //For Drag and Drop

  ui->doubleSpinBox_rf_freq->setValue(glob_settings.glob_args.eyeArgs.rf_freq/(1000*1000));
  ui->checkBox_enable_shortcut->setChecked(glob_settings.glob_args.eyeArgs.enable_shortcut_discovery);
}

MainWindow::~MainWindow() {
  eyeThread.stop();
  eyeThread.unsubscribe(&spectrumAdapter);
  delete cp;
  delete ui;
}

void MainWindow::on_actionStart_triggered() {
  if(active_eye) {
    qDebug() << "Window exists";
  }
  else {

    active_eye = true;

    // Setup prog args (from GUI)
    eyeArgs.file_nof_ports  = static_cast<uint32_t>(ui->spinBox_Ports->value());
    eyeArgs.file_cell_id    = ui->spinBox_CellId->value();
    eyeArgs.file_nof_prb    = ui->spinBox_Prb->value();

    if(ui->checkBox_FileAsSource->isChecked()){
      QString filename = ui->lineEdit_FileName->text();
      if(!get_args_from_file(filename)) {
        qDebug() << "Could not load parameters from file source" << endl;
        return;
      }
    }
    else {
      eyeArgs.input_file_name = "";
    }

    qDebug() << "RF_Freq: "<< eyeArgs.rf_freq;

    //Init Adapters:

    spectrumAdapter.emit_uplink     = false;
    spectrumAdapter.emit_downlink   = false;
    spectrumAdapter.emit_spectrum   = false;
    spectrumAdapter.emit_difference = false;

    //Start Windows:

    //Uplink:
    if(glob_settings.glob_args.gui_args.show_uplink){
      ul_alloc->activate();
    }

    //Downlink:
    if(glob_settings.glob_args.gui_args.show_downlink){
      dl_alloc->activate();
    }

    // Pure Spectrum:
    if(glob_settings.glob_args.gui_args.show_spectrum){
      dl_spec->activate();
    }

    // Spectrum - Downlink
    if(glob_settings.glob_args.gui_args.show_diff){
      diff_alloc->activate();
    }

    // Performance Plot extern:
    if(glob_settings.glob_args.gui_args.show_performance_plot){
      perf_plot->activate();
    }

    // RNTI Table:
    if(glob_settings.glob_args.gui_args.show_rnti){
      rnti_table->activate();
    }

    // Organise Windows:

    ui->mdiArea->tileSubWindows();

    eyeThread.start(eyeArgs);
    // Emit signal to initially set rnti histogram threshold
    emit(on_slider_hist_threshold_valueChanged(ui->slider_hist_threshold->value()));
    ui->doubleSpinBox_rf_freq->setValue(eyeArgs.rf_freq/(1000*1000));
    ui->spinBox_Ports->setValue(eyeArgs.file_nof_ports);
    ui->spinBox_CellId->setValue(eyeArgs.file_cell_id);
    ui->spinBox_Prb->setValue(eyeArgs.file_nof_prb);

    qDebug() << "Spectrum View on";
    glob_settings.store_settings();
  }
  ui->spinBox_nof_sf_workers->setEnabled(false);
}

void MainWindow::on_actionStop_triggered()
{
  eyeThread.stop();
  if(perf_plot != nullptr) perf_plot->deactivate();
  if(ul_alloc != nullptr) ul_alloc->deactivate();
  if(dl_alloc != nullptr) dl_alloc->deactivate();
  if(diff_alloc != nullptr) diff_alloc->deactivate();
  if(dl_spec != nullptr) dl_spec->deactivate();
  if(rnti_table != nullptr) rnti_table->deactivate();
  active_eye = false;
  spectrumAdapter.disconnect();  //Disconnect all Signals
  ui->spinBox_nof_sf_workers->setEnabled(true);
}

void MainWindow::on_Select_file_button_clicked()
{
  qDebug () << "Clicked Select File";
  FileDialog input_file;
  ui->lineEdit_FileName->setText(input_file.openFile());
  on_lineEdit_FileName_editingFinished();
}

void MainWindow::on_lineEdit_FileName_textChanged(const QString &arg1)
{

  QString buffer_string;

  buffer_string = ui->lineEdit_FileName->text();

  if(buffer_string.contains("file://")){

    buffer_string.remove("file://");
    ui->lineEdit_FileName->setText(buffer_string);
  }

  //qDebug() <<"Buffer String: "<< buffer_string;

  if(glob_settings.glob_args.gui_args.use_file_as_source) {
    get_args_from_file(buffer_string);
  }

}

void MainWindow::update_cell_config_fields() {
  ui->spinBox_CellId->setValue(static_cast<int>(eyeArgs.file_cell_id));
  ui->doubleSpinBox_rf_freq->setValue(eyeArgs.rf_freq / (1000*1000));
  ui->spinBox_Prb->setValue(eyeArgs.file_nof_prb);
}

bool MainWindow::get_args_from_file(const QString filename) {

  qDebug() << "Filename: " << filename;

  bool no_proberesult = false;
  bool no_networkinfo = false;

  QString basename = filename;
  if(basename.contains("-iq.bin") > 0) {
    basename.remove("-iq.bin");
  }
  else if(basename.contains("-traffic.csv") > 0) {
    basename.remove("-traffic.csv");
  }
  else if(basename.contains("-cell.csv") > 0) {
    basename.remove("-cell.csv");
  }

  QString probeResultFilename = basename + "-traffic.csv";
  QString cellInfoFilename = basename + "-cell.csv";
  QString iqSamplesFilename = basename + "-iq.bin";

  qDebug() << "probeResultFilename: " << probeResultFilename;
  qDebug() << "cellInfoFilename: " << cellInfoFilename;
  qDebug() << "iqSamplesFilename: " << iqSamplesFilename;

  ProbeResult probeResult;
  QFile probeResultFile(probeResultFilename);
  if(probeResultFile.open(QIODevice::ReadOnly)) {
    QTextStream linestream(&probeResultFile);
    //while(!linestream.atEnd()) {
    if(!linestream.atEnd()) { // no loop, only first line
      QString line = linestream.readLine();
      probeResult.fromCSV(line.toStdString(), ',');
      qDebug() << QString::fromStdString(probeResult.toCSV(','));
    }
    probeResultFile.close();
  }
  else {
    qDebug () << "Could not open probeResultFile: " << probeResultFilename << endl;
    no_proberesult = true;
  }

  NetworkInfo networkInfo;
  QFile cellInfoFile(cellInfoFilename);
  if(cellInfoFile.open(QIODevice::ReadOnly)) {
    QTextStream linestream(&cellInfoFile);
    //while(!linestream.atEnd()) {
    if(!linestream.atEnd()) { // no loop, only first line
      QString line = linestream.readLine();
      networkInfo.fromCSV(line.toStdString(), ',');
      qDebug() << QString::fromStdString(networkInfo.toCSV(','));
    }
    cellInfoFile.close();
  }
  else {
    qDebug () << "Could not open cellInfoFile: " << cellInfoFilename << endl;
    no_networkinfo = true;
  }

  eyeArgs.input_file_name = iqSamplesFilename.toLatin1().constData();
  if(!no_networkinfo){
    eyeArgs.file_cell_id = static_cast<uint32_t>(networkInfo.lteinfo->pci);
    eyeArgs.rf_freq = networkInfo.rf_freq; // rf_freq is in MHz, need Hz
    eyeArgs.file_nof_prb = networkInfo.nof_prb;
  }
  if(!no_proberesult){
  }

  update_cell_config_fields();

  return true;
}

void MainWindow::wheelEvent(QWheelEvent *event){
  // As the wheel event can only be captured in main window, forward the delta to waterfall's wheelEvent method
  if (ui->mdiArea->underMouse()){
    if(dl_alloc != nullptr){dl_alloc->wheelEvent(event->delta());}
    if(ul_alloc != nullptr){ul_alloc->wheelEvent(event->delta());}
    if(diff_alloc != nullptr){diff_alloc->wheelEvent(event->delta());}
    if(dl_spec != nullptr){dl_spec->wheelEvent(event->delta());}
  }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
  if (e->mimeData()->hasUrls()) {
    e->acceptProposedAction();
  }
}

void MainWindow::dropEvent(QDropEvent *e)
{
  foreach (const QUrl &url, e->mimeData()->urls()) {
    QString fileName = url.toLocalFile();
    ui->lineEdit_FileName->setText(fileName);
  }
}

void MainWindow::on_actionTile_Windows_triggered() {
  ui->mdiArea->tileSubWindows();
}

void MainWindow::on_spinBox_Prb_valueChanged(int value) {
  eyeArgs.file_nof_prb = static_cast<uint32_t>(value);
}
void MainWindow::on_pushButton_uplink_color_clicked(){cp->on_pushButton_uplink_color_clicked();}
void MainWindow::on_pushButton_downlink_color_clicked(){cp->on_pushButton_downlink_color_clicked();}
