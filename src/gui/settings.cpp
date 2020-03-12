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
#include "settings.h"
#include <QWidget>
#include <QDebug>
#include <QSettings>

#include <iostream>

Settings::Settings() : glob_args() {

  //Init decoder args:
  ArgManager::defaultArgs(glob_args.eyeArgs);

  //TODO: This should be set by a checkbox
  //glob_args.eyeArgs.skip_secondary_meta_formats = true;

  //Load settings or fill+store defaults
  settings = new QSettings(QSettings::IniFormat,QSettings::UserScope,"CNI","FalconGui");

  settings->beginGroup("GUI");
  if(settings->contains("SAVE_TO_FILE")){//File Exists, load settings:

    settings->endGroup();
    load_settings();
  }
  else { //File doesn't exist, create one with default settings:
    settings->endGroup();

    //Default Settings:
    //GUI args:
    glob_args.gui_args.path_to_file         = nullptr;
    glob_args.gui_args.save_settings        = true;
    glob_args.gui_args.show_diff            = false;
    glob_args.gui_args.show_downlink        = true;
    glob_args.gui_args.show_spectrum        = false;
    glob_args.gui_args.show_uplink          = true;
    glob_args.gui_args.use_file_as_source   = true;
    glob_args.gui_args.show_performance_plot= false;
    glob_args.gui_args.show_rnti = false;
    glob_args.gui_args.sort_by_column = 0;
    glob_args.gui_args.sort_order = 0;
    glob_args.gui_args.perf_fps = 4;
    glob_args.gui_args.wf_fps = 60;
    glob_args.eyeArgs.nof_subframe_workers = 20;

    // glob_args.gui_args.show_plot_downlink   = false;

    //Spectrum args:
    glob_args.spectrum_args.mouse_wheel_sens    = 100;
    glob_args.spectrum_args.spectrum_line_count = 300;
    glob_args.spectrum_args.spectrum_line_shown = 100;
    glob_args.spectrum_args.spectrum_line_width = 50;

    store_settings(); //Default settings on startup are saved to file.
  }

}

Settings::~Settings() {
  delete settings;
}

void Settings::load_settings() {

  //Load GUI Settings:

  settings->beginGroup("GUI");

  glob_args.gui_args.save_settings        = settings->value("SAVE_TO_FILE").toBool();
  glob_args.gui_args.show_diff            = settings->value("SHOW_DIFF").toBool();
  glob_args.gui_args.show_downlink        = settings->value("SHOW_DOWNLINK").toBool();
  glob_args.gui_args.show_spectrum        = settings->value("SHOW_SPECTRUM").toBool();
  glob_args.gui_args.show_uplink          = settings->value("SHOW_UPLINK").toBool();
  glob_args.gui_args.use_file_as_source   = settings->value("USE_FILE_AS_SOURCE").toBool();
  glob_args.gui_args.show_performance_plot= settings->value("SHOW_PERFORMANCE_PLOT").toBool();
  glob_args.gui_args.show_rnti            = settings->value("SHOW_RNTI").toBool();
  //glob_args.gui_args.show_plot_downlink   = settings->value("SHOW_DOWNLINK_PLOT").toBool();
  settings->endGroup();

  //Load Spectrum Settings:

  settings->beginGroup("Spectrum");

  glob_args.spectrum_args.mouse_wheel_sens     = settings->value("MOUSE_WHEEL_SENS").toInt();
  glob_args.spectrum_args.spectrum_line_count  = settings->value("SPECTRUM_LINE_COUNT").toInt();
  glob_args.spectrum_args.spectrum_line_shown  = settings->value("SPECTRUM_LINE_SHOWN").toInt();
  glob_args.spectrum_args.spectrum_line_width  = settings->value("SPECTRUM_LINE_WIDTH").toInt();
  glob_args.gui_args.wf_fps   = settings->value("WF_FPS").toInt();


  settings->endGroup();

  settings->beginGroup("PERFORMANCE_PLOTS");
  glob_args.gui_args.perf_fps   = settings->value("PERF_FPS").toInt();
  settings->endGroup();

  //Load EYE Settings:

  settings->beginGroup("EYE");

  glob_args.eyeArgs.rf_freq          = settings->value("RF_FREQ").toDouble();
  glob_args.gui_args.path_to_file    = settings->value("PATH_TO_FILE").toString().toLocal8Bit();
  glob_args.eyeArgs.enable_shortcut_discovery = settings->value("ENABLE_SHORTCUTDISCOVERY").toBool();
  glob_args.eyeArgs.nof_subframe_workers = settings->value("NOF_SUBFRAME_WORKERS").toInt();

  settings->endGroup();

  // Load RNTI table settings:

  settings->beginGroup("RNTI_TABLE");
  glob_args.gui_args.sort_by_column = settings->value("SORT_BY_COLUMN").toInt();
  glob_args.gui_args.sort_order = settings->value("SORT_ORDER").toInt();
  settings->endGroup();

}

void Settings::store_settings(){

  //Save GUI Settings:

  settings->beginGroup("GUI");

  settings->setValue("SAVE_TO_FILE"         , glob_args.gui_args.save_settings);
  settings->setValue("SHOW_DIFF"            , glob_args.gui_args.show_diff);
  settings->setValue("SHOW_DOWNLINK"        , glob_args.gui_args.show_downlink);
  settings->setValue("SHOW_SPECTRUM"        , glob_args.gui_args.show_spectrum);
  settings->setValue("SHOW_UPLINK"          , glob_args.gui_args.show_uplink);
  settings->setValue("USE_FILE_AS_SOURCE"   , glob_args.gui_args.use_file_as_source);
  settings->setValue("SHOW_PERFORMANCE_PLOT", glob_args.gui_args.show_performance_plot);
  settings->setValue("SHOW_RNTI"            , glob_args.gui_args.show_rnti );
  //settings->setValue("SHOW_DOWNLINK_PLOT" , glob_args.gui_args.show_plot_downlink);

  settings->endGroup();

  //Save Spectrum Settings:

  settings->beginGroup("Spectrum");

  settings->setValue("MOUSE_WHEEL_SENS"       , glob_args.spectrum_args.mouse_wheel_sens);
  settings->setValue("SPECTRUM_LINE_COUNT"    , glob_args.spectrum_args.spectrum_line_count);
  settings->setValue("SPECTRUM_LINE_SHOWN"    , glob_args.spectrum_args.spectrum_line_shown);
  settings->setValue("SPECTRUM_LINE_WIDTH"    , glob_args.spectrum_args.spectrum_line_width);
  settings->setValue("WF_FPS"     ,glob_args.gui_args.wf_fps);

  settings->endGroup();

  settings->beginGroup("PERFORMANCE_PLOTS");
  settings->setValue("PERF_FPS", glob_args.gui_args.perf_fps);
  settings->endGroup();

  //Save EYE Settings:

  settings->beginGroup("EYE");

  settings->setValue("RF_FREQ"            , glob_args.eyeArgs.rf_freq);
  settings->setValue("PATH_TO_FILE"       , glob_args.gui_args.path_to_file);
  settings->setValue("ENABLE_SHORTCUTDISCOVERY", glob_args.eyeArgs.enable_shortcut_discovery);
  settings->setValue("NOF_SUBFRAME_WORKERS", glob_args.eyeArgs.nof_subframe_workers);

  settings->endGroup();

  // Load RNTI table settings:
  settings->beginGroup("RNTI_TABLE");
  settings->setValue("SORT_BY_COLUMN" , glob_args.gui_args.sort_by_column);
  settings->setValue("SORT_ORDER", glob_args.gui_args.sort_order);
  settings->endGroup();

}
