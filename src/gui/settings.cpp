#include "settings.h"
#include <QWidget>
#include <QDebug>
#include <QSettings>

Settings::Settings(){

    //Load settings or store default:

    settings = new QSettings(QSettings::IniFormat,QSettings::UserScope,"CNI","FalconGui");

    settings->beginGroup("GUI");
    if(settings->contains("SAVE_TO_FILE")){//File Exists, load settings:

        settings->endGroup();
        load_settings();
    }
    else{ //File doesn't exist, create one with default settings:
        settings->endGroup();
        //Default Settings:
        //GUI args:

        glob_args.gui_args.path_to_file         = "";
        glob_args.gui_args.save_settings        = true;
        glob_args.gui_args.show_diff            = true;
        glob_args.gui_args.show_downlink        = true;
        glob_args.gui_args.show_spectrum        = true;
        glob_args.gui_args.show_uplink          = true;
        glob_args.gui_args.use_file_as_source   = true;

        //Decode args:

        glob_args.decoder_args.rf_freq = 0;

        //Spectrum args:

        glob_args.spectrum_args.mouse_wheel_sens    = 100;
        glob_args.spectrum_args.spectrum_line_count = 300;
        glob_args.spectrum_args.spectrum_line_shown = 100;
        glob_args.spectrum_args.spectrum_line_width = 50;

        store_settings(); //Default settings on startup are saved to file.
    }

}

Settings::~Settings(){

    delete settings;
}

void Settings::load_settings(){

    //Load GUI Settings:

    settings->beginGroup("GUI");

    glob_args.gui_args.save_settings        = settings->value("SAVE_TO_FILE").toBool();
    glob_args.gui_args.show_diff            = settings->value("SHOW_DIFF").toBool();
    glob_args.gui_args.show_downlink        = settings->value("SHOW_DOWNLINK").toBool();
    glob_args.gui_args.show_spectrum        = settings->value("SHOW_SPECTRUM").toBool();
    glob_args.gui_args.show_uplink          = settings->value("SHOW_UPLINK").toBool();
    glob_args.gui_args.use_file_as_source   = settings->value("USE_FILE_AS_SOURCE").toBool();

    settings->endGroup();

    //Load Spectrum Settings:

    settings->beginGroup("Spectrum");

    glob_args.spectrum_args.mouse_wheel_sens     = settings->value("MOUSE_WHEEL_SENS").toInt();
    glob_args.spectrum_args.spectrum_line_count  = settings->value("SPECTRUM_LINE_COUNT").toInt();
    glob_args.spectrum_args.spectrum_line_shown  = settings->value("SPECTRUM_LINE_SHOWN").toInt();
    glob_args.spectrum_args.spectrum_line_width  = settings->value("SPECTRUM_LINE_WIDTH").toInt();

    settings->endGroup();

    //Load EYE Settings:

    settings->beginGroup("EYE");

    glob_args.decoder_args.rf_freq          = settings->value("RF_FREQ").toDouble();
    //glob_args.decoder_args.input_file_name  = settings->value("PATH_TO_FILE").toString().toLocal8Bit();

    settings->endGroup();

}

void Settings::store_settings(){

    //Save GUI Settings:

    settings->beginGroup("GUI");

    settings->setValue("SAVE_TO_FILE"       , glob_args.gui_args.save_settings);
    settings->setValue("SHOW_DIFF"          , glob_args.gui_args.show_diff);
    settings->setValue("SHOW_DOWNLINK"      , glob_args.gui_args.show_downlink);
    settings->setValue("SHOW_SPECTRUM"      , glob_args.gui_args.show_spectrum);
    settings->setValue("SHOW_UPLINK"        , glob_args.gui_args.show_uplink);
    settings->setValue("USE_FILE_AS_SOURCE" , glob_args.gui_args.use_file_as_source);

    settings->endGroup();

    //Save Spectrum Settings:

    settings->beginGroup("Spectrum");

    settings->setValue("MOUSE_WHEEL_SENS"       , glob_args.spectrum_args.mouse_wheel_sens);
    settings->setValue("SPECTRUM_LINE_COUNT"    , glob_args.spectrum_args.spectrum_line_count);
    settings->setValue("SPECTRUM_LINE_SHOWN"    , glob_args.spectrum_args.spectrum_line_shown);
    settings->setValue("SPECTRUM_LINE_WIDTH"    , glob_args.spectrum_args.spectrum_line_width);

    settings->endGroup();

    //Save EYE Settings:

    settings->beginGroup("EYE");

    settings->setValue("RF_FREQ"            , glob_args.decoder_args.rf_freq);
    settings->setValue("PATH_TO_FILE"       , glob_args.decoder_args.input_file_name);

    settings->endGroup();


}
