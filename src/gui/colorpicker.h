#ifndef COLORPICKER_H
#define COLORPICKER_H

#pragma once

#include "settings.h"
#include "ui_mainwindow.h"  // for valid usage of Ui::MainWindow
#include "rangewidget/RangeWidget.h"
#include "spectrum.h"
#include <QColorDialog>


class Colorpicker : public QObject{
    Q_OBJECT
public:
    explicit Colorpicker(Settings* p_glob_settings, Ui::MainWindow* p_ui);
    ~Colorpicker();

    RangeWidget* get_color_range_slider();

private:
    Settings* glob_settings;
    Ui::MainWindow *ui;

    bool downlink_color_active;
    QColorDialog *color_dialog = nullptr;
    RangeWidget *color_range_slider = nullptr;
    QPalette downlink_palette;
    QPalette uplink_palette;

    QLabel* slider_label = nullptr;

    QColor dl_old;
    QColor ul_old;

    Spectrum *spectrum_view      = nullptr;

 public slots:
    void on_pushButton_downlink_color_clicked();
    void on_pushButton_uplink_color_clicked();
    void set_color(const QColor &color);

private slots:
    void restore_color();

signals:
    void color_change();


};



#endif // COLORPICKER_H
