#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QFile>
#include "model_dummy/cni_cc_decoder.h"
#include <QSettings>

typedef struct{
    //bool settings:
    bool save_settings;
    bool use_file_as_source;
    bool show_spectrum;
    bool show_diff;
    bool show_uplink;
    bool show_downlink;

    //String settings:
    QString path_to_file;

}Gui_SettingsType_t;

typedef struct{

    //int settings:
    int mouse_wheel_sens;
    int spectrum_line_count;
    int spectrum_line_width;
    int spectrum_line_shown;

}Spectrum_SettingsType_t;

typedef struct {

    Gui_SettingsType_t      gui_args;
    Spectrum_SettingsType_t spectrum_args;
    prog_args_t             decoder_args;

}Global_SettingsType_t;


/*

typedef enum{

    //bool settings:

    SAVE_SETTINGS           = 0,
    USE_FILE_AS_SOURCE      = 1,
    SHOW_SPECTRUM           = 2,
    SHOW_DIFF               = 3,
    SHOW_UPLINK             = 4,
    SHOW_DOWNLINK           = 5,
    MOUSE_WHEEL_SENS        = 6,
    SPECTRUM_LINE_COUNT     = 7,
    SPECTRUM_LINE_WIDTH     = 8,
    SPECTRUM_LINE_SHOWN     = 9,
    RF_FREQ                 = 10,
    PATH_TO_FILE            = 11

}SettingsType_t;
*/

class Settings
{

private:

    QSettings *settings;

public:

    Global_SettingsType_t glob_args;

    explicit Settings();
    virtual ~Settings();

    void load_settings();
    void store_settings();

};



#endif // SETTINGS_H
