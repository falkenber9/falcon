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

  //Init GUI args with default settings
  default_settings();

  //Init decoder args:
  ArgManager::defaultArgs(glob_args.eyeArgs);

  //TODO: This should be set by a checkbox
  //glob_args.eyeArgs.skip_secondary_meta_formats = true;

  //Load settings or fill+store defaults
  settings = new QSettings(QSettings::IniFormat,QSettings::UserScope,"CNI","FalconGui");
  load_settings();

//  settings->beginGroup("GUI");
//  if(settings->contains("SAVE_TO_FILE")){//File exists, load remaining settings
//    settings->endGroup();
//    load_settings();
//  }
//  else { //File doesn't exist, create one with default settings:
//    settings->endGroup();
//    store_settings(); //Default settings on startup are saved to file.
//  }

}

Settings::~Settings() {
  delete settings;
}

void Settings::default_settings() {
  //Default Settings:
  //GUI args:
  glob_args.gui_args.path_to_file         = nullptr;
  glob_args.gui_args.save_settings        = true;
  glob_args.gui_args.show_diff            = false;
  glob_args.gui_args.show_downlink        = true;
  glob_args.gui_args.show_spectrum        = false;
  glob_args.gui_args.show_uplink          = true;
  glob_args.gui_args.use_file_as_source   = true;
  glob_args.gui_args.show_cell_activity   = false;
  glob_args.gui_args.show_ue_activity     = false;
  glob_args.gui_args.show_affiliation     = true;
  glob_args.gui_args.show_institute       = true;
  glob_args.gui_args.show_funding         = false;
  glob_args.gui_args.show_banner          = true;
  glob_args.gui_args.show_advanced        = false;
  glob_args.gui_args.sort_by_column       = 0;
  glob_args.gui_args.sort_order           = 0;
  glob_args.gui_args.perf_fps             = 4;
  glob_args.gui_args.wf_fps               = 60;

  // glob_args.gui_args.show_plot_downlink   = false;

  //Spectrum args:
  glob_args.spectrum_args.mouse_wheel_sens    = 100;
  glob_args.spectrum_args.spectrum_line_count = 300;
  glob_args.spectrum_args.spectrum_line_shown = 100;
  glob_args.spectrum_args.spectrum_line_width = 50;

  glob_args.eyeArgs.nof_subframe_workers = 20;
}

void Settings::load_settings() {

  //Load GUI Settings:
  settings->beginGroup("GUI");
  glob_args.gui_args.save_settings        = settings->value("SAVE_TO_FILE", glob_args.gui_args.save_settings).toBool();
  glob_args.gui_args.show_diff            = settings->value("SHOW_DIFF", glob_args.gui_args.show_diff).toBool();
  glob_args.gui_args.show_downlink        = settings->value("SHOW_DOWNLINK", glob_args.gui_args.show_downlink).toBool();
  glob_args.gui_args.show_spectrum        = settings->value("SHOW_SPECTRUM", glob_args.gui_args.show_spectrum).toBool();
  glob_args.gui_args.show_uplink          = settings->value("SHOW_UPLINK", glob_args.gui_args.show_uplink).toBool();
  glob_args.gui_args.use_file_as_source   = settings->value("USE_FILE_AS_SOURCE", glob_args.gui_args.use_file_as_source).toBool();
  glob_args.gui_args.show_cell_activity   = settings->value("SHOW_CELL_ACTIVITY", glob_args.gui_args.show_cell_activity).toBool();
  glob_args.gui_args.show_ue_activity     = settings->value("SHOW_UE_ACTIVITY", glob_args.gui_args.show_ue_activity).toBool();
  glob_args.gui_args.show_affiliation     = settings->value("SHOW_AFFILIATION", glob_args.gui_args.show_affiliation).toBool();
  glob_args.gui_args.show_institute       = settings->value("SHOW_INSTITUTE", glob_args.gui_args.show_institute).toBool();
  glob_args.gui_args.show_funding         = settings->value("SHOW_FUNDING", glob_args.gui_args.show_funding).toBool();
  glob_args.gui_args.show_banner          = settings->value("SHOW_BANNER", glob_args.gui_args.show_banner).toBool();
  glob_args.gui_args.show_advanced        = settings->value("SHOW_ADVANCED", glob_args.gui_args.show_advanced).toBool();
  //glob_args.gui_args.show_plot_downlink   = settings->value("SHOW_DOWNLINK_PLOT", glob_args.gui_args.show_plot_downlink).toBool();
  settings->endGroup();

  // Load UE Activity Settings:
  settings->beginGroup("UE_ACTIVITY");
  glob_args.gui_args.sort_by_column = settings->value("SORT_BY_COLUMN", glob_args.gui_args.sort_by_column).toInt();
  glob_args.gui_args.sort_order     = settings->value("SORT_ORDER", glob_args.gui_args.sort_order).toInt();
  settings->endGroup();

  // Load Cell Activity Settings:
  settings->beginGroup("CELL_ACTIVITY");
  glob_args.gui_args.perf_fps   = settings->value("PERF_FPS", glob_args.gui_args.perf_fps).toInt();
  settings->endGroup();

  //Load Spectrum Settings:
  settings->beginGroup("SPECTRUM");
  glob_args.gui_args.wf_fps                    = settings->value("WF_FPS", glob_args.gui_args.wf_fps).toInt();
  glob_args.spectrum_args.mouse_wheel_sens     = settings->value("MOUSE_WHEEL_SENS", glob_args.spectrum_args.mouse_wheel_sens).toInt();
  glob_args.spectrum_args.spectrum_line_count  = settings->value("SPECTRUM_LINE_COUNT", glob_args.spectrum_args.spectrum_line_count).toInt();
  glob_args.spectrum_args.spectrum_line_shown  = settings->value("SPECTRUM_LINE_SHOWN", glob_args.spectrum_args.spectrum_line_shown).toInt();
  glob_args.spectrum_args.spectrum_line_width  = settings->value("SPECTRUM_LINE_WIDTH", glob_args.spectrum_args.spectrum_line_width).toInt();
  settings->endGroup();

  //Load EYE Settings:
  settings->beginGroup("EYE");
  glob_args.eyeArgs.rf_freq                     = settings->value("RF_FREQ", glob_args.eyeArgs.rf_freq).toDouble();
  glob_args.gui_args.path_to_file               = settings->value("PATH_TO_FILE", glob_args.gui_args.path_to_file).toString().toLocal8Bit();
  glob_args.eyeArgs.enable_shortcut_discovery   = settings->value("ENABLE_SHORTCUTDISCOVERY", glob_args.eyeArgs.enable_shortcut_discovery).toBool();
  glob_args.eyeArgs.nof_subframe_workers        = settings->value("NOF_SUBFRAME_WORKERS", glob_args.eyeArgs.nof_subframe_workers).toInt();
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
  settings->setValue("SHOW_CELL_ACTIVITY"   , glob_args.gui_args.show_cell_activity);
  settings->setValue("SHOW_UE_ACTIVITY"     , glob_args.gui_args.show_ue_activity);
  settings->setValue("SHOW_AFFILIATION"     , glob_args.gui_args.show_affiliation);
  settings->setValue("SHOW_INSTITUTE"       , glob_args.gui_args.show_institute);
  settings->setValue("SHOW_FUNDING"         , glob_args.gui_args.show_funding);
  settings->setValue("SHOW_BANNER"          , glob_args.gui_args.show_banner);
  settings->setValue("SHOW_ADVANCED"        , glob_args.gui_args.show_advanced);
  //settings->setValue("SHOW_DOWNLINK_PLOT" , glob_args.gui_args.show_plot_downlink);
  settings->endGroup();

  // Save UE Activity Settings:
  settings->beginGroup("UE_ACTIVITY");
  settings->setValue("SORT_BY_COLUMN" , glob_args.gui_args.sort_by_column);
  settings->setValue("SORT_ORDER"     , glob_args.gui_args.sort_order);
  settings->endGroup();

  // Save Cell Activity Settings:
  settings->beginGroup("CELL_ACTIVITY");
  settings->setValue("PERF_FPS", glob_args.gui_args.perf_fps);
  settings->endGroup();

  //Save Spectrum Settings:
  settings->beginGroup("SPECTRUM");
  settings->setValue("WF_FPS"                 , glob_args.gui_args.wf_fps);
  settings->setValue("MOUSE_WHEEL_SENS"       , glob_args.spectrum_args.mouse_wheel_sens);
  settings->setValue("SPECTRUM_LINE_COUNT"    , glob_args.spectrum_args.spectrum_line_count);
  settings->setValue("SPECTRUM_LINE_SHOWN"    , glob_args.spectrum_args.spectrum_line_shown);
  settings->setValue("SPECTRUM_LINE_WIDTH"    , glob_args.spectrum_args.spectrum_line_width);
  settings->endGroup();

  //Save EYE Settings:
  settings->beginGroup("EYE");
  settings->setValue("RF_FREQ"                  , glob_args.eyeArgs.rf_freq);
  settings->setValue("PATH_TO_FILE"             , glob_args.gui_args.path_to_file);
  settings->setValue("ENABLE_SHORTCUTDISCOVERY" , glob_args.eyeArgs.enable_shortcut_discovery);
  settings->setValue("NOF_SUBFRAME_WORKERS"     , glob_args.eyeArgs.nof_subframe_workers);
  settings->endGroup();
}
