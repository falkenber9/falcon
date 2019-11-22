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
#pragma once

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QFile>
//#include "model_dummy/cni_cc_decoder.h"
#include "eye/ArgManager.h"
#include <QSettings>

typedef struct {
    //bool settings:
    bool save_settings;
    bool use_file_as_source;
    bool show_spectrum;
    bool show_diff;
    bool show_uplink;
    bool show_downlink;
    bool show_performance_plot;
    //bool show_plot_downlink;

    //String settings:
    QString path_to_file;

    //Colors:
    QColor downlink_plot_color;
    QColor uplink_plot_color;

    //Int Values:
    int spectrum_color_start;
    int spectrum_color_stop;
} Gui_SettingsType_t;

typedef struct {
    //int settings:
    int mouse_wheel_sens;
    int spectrum_line_count;
    int spectrum_line_width;
    int spectrum_line_shown;
} Spectrum_SettingsType_t;

typedef struct {
    Gui_SettingsType_t      gui_args;
    Spectrum_SettingsType_t spectrum_args;
    Args  eyeArgs;
    //prog_args_t             decoder_args;

} Global_SettingsType_t;

class Settings {
private:
    QSettings *settings;

public:
    Global_SettingsType_t glob_args;
    Args test_args;
    std::string test_string;

    explicit Settings();
    virtual ~Settings();

    void load_settings();
    void store_settings();
};
