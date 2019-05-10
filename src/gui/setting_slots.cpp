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

void MainWindow::on_spinBox_rf_freq_editingFinished()//RF-Freq changed:
{
    glob_settings.glob_args.decoder_args.rf_freq = ui->spinBox_rf_freq->value();   //Save Value to glob_args
    ui->lcdNumber_rf_freq->display(glob_settings.glob_args.decoder_args.rf_freq);  //Display rf_freq

    if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.
}

void MainWindow::on_checkBox_FileAsSource_clicked()
{

    glob_settings.glob_args.gui_args.use_file_as_source = ui->checkBox_FileAsSource->isChecked(); //Store checkbox Flag

    if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.
}

void MainWindow::on_lineEdit_FileName_editingFinished()
{

    glob_settings.glob_args.gui_args.path_to_file = ui->lineEdit_FileName->text().toLatin1().data(); //Store line as path to file.

    if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.

}

void MainWindow::on_actionUse_File_as_Source_changed()
{
    glob_settings.glob_args.gui_args.use_file_as_source = ui->actionUse_File_as_Source->isChecked(); //Store checkbox Flag

    if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.
}

void MainWindow::on_actionSpectrum_changed()
{
    glob_settings.glob_args.gui_args.show_spectrum = ui->actionSpectrum->isChecked();

    if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.
}

void MainWindow::on_actionDifference_changed()
{
    glob_settings.glob_args.gui_args.show_diff = ui->actionDifference->isChecked();

    if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.
}

void MainWindow::on_actionUplink_changed()
{
    glob_settings.glob_args.gui_args.show_uplink = ui->actionUplink->isChecked();

    if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.
}

void MainWindow::on_actionDownlink_changed()
{
    glob_settings.glob_args.gui_args.show_downlink = ui->actionDownlink->isChecked();

    if(glob_settings.glob_args.gui_args.save_settings)glob_settings.store_settings();  //If save_settings = true, save to file.
}

void MainWindow::on_actionSave_Settings_changed()
{
    glob_settings.glob_args.gui_args.save_settings = ui->actionSave_Settings->isChecked();

    glob_settings.store_settings();  //save to file once.
}

void MainWindow::on_actionplot1_changed()
{
    show_plot1 = ui->actionplot1->isChecked();
}


