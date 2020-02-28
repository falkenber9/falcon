#ifndef WATERFALL_H
#define WATERFALL_H

#pragma once

#include "qcustomplot/qcustomplot.h"
#include "settings.h"
#include "falcon/CCSnifferInterfaces.h"
#include "spectrum.h"
#include "adapters_qt/SpectrumAdapter.h"
#include <functional>

//#include "plots.h"

class Waterfall : public QObject{
  Q_OBJECT
public:
  // Handle pointers on Main windows settings
    explicit Waterfall(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea);
    ~Waterfall();

    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void decorate() = 0;

    void wheelEvent(int delta);

protected:
    bool active = false;
    Settings* glob_settings = nullptr;
    SpectrumAdapter* spectrumAdapter = nullptr;
    QMdiArea* mdiArea = nullptr;

    Spectrum *spectrum_view   = nullptr;
    QWidget *m_window = nullptr;
    QMdiSubWindow *m_subwindow = nullptr;

protected slots:
    void SubWindow_mousePressEvent();
    void draw(const ScanLineLegacy*);
    void setFPS(int fps);
};

class Waterfall_UL : public Waterfall{
  Q_OBJECT
public:
  Waterfall_UL(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea);
  ~Waterfall_UL();
  virtual void activate() override;
  virtual void deactivate() override;
  virtual void decorate() override;
};

class Waterfall_DL : public Waterfall{
  Q_OBJECT
public:
  Waterfall_DL(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea);
  ~Waterfall_DL();
  virtual void activate() override;
  virtual void deactivate() override;
  virtual void decorate() override;
};

class Waterfall_DIFF : public Waterfall{
  Q_OBJECT
public:
  Waterfall_DIFF(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea);
  ~Waterfall_DIFF();
  virtual void activate() override;
  virtual void deactivate() override;
  virtual void decorate() override;
};

class Waterfall_SPEC : public Waterfall{
  Q_OBJECT
public:
  Waterfall_SPEC(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea);
  ~Waterfall_SPEC();
  virtual void activate() override;
  virtual void deactivate() override;
  virtual void decorate() override;
  void range_slider_value_changed(int _1st, int _2nd);
};

#endif // WATERFALL_H
