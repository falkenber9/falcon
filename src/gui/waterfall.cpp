#include "waterfall.h"

// Initial state of spectra
bool Spectrum::paused = false;
int Spectrum::scroll_offset = 0;

Waterfall::Waterfall(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea) :
  glob_settings(p_glob_settings),
  spectrumAdapter(p_spectrumAdapter),
  mdiArea(p_mdiArea){
  // Initialize objects
  m_window = new QWidget();
  spectrum_view = new Spectrum(m_window, glob_settings);
  // Register widget in subwindow
  m_subwindow = mdiArea->addSubWindow(m_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  // Maximize widget and use size to resize spectrum view
  m_subwindow->showMaximized();
  spectrum_view->setFixedSize(m_window->size().width(),m_window->size().height());
  // Finally show widget
  m_window->show();
}

Waterfall::~Waterfall(){
  if(mdiArea != nullptr){ // mdiArea != nullptr --> was activated at least once
    //Close Subwindow:
    mdiArea->setActiveSubWindow(m_subwindow);
    mdiArea->closeActiveSubWindow();
    mdiArea->removeSubWindow(m_window);
  }
}

void Waterfall::draw(const ScanLineLegacy *data){
  if (glob_settings->glob_args.spectrum_args.spectrum_line_width != data->l_prb){
    glob_settings->glob_args.spectrum_args.spectrum_line_width = data->l_prb;
  }
  if(active) {
    spectrum_view->addLine(data->linebuf);

    //Autoscaling for Spectrum
    if(m_window->size().height() != spectrum_view->height() ||
       m_window->size().width() != spectrum_view->width() )
    {
      spectrum_view->setFixedSize(m_window->size().width(),m_window->size().height());
    }
  }
  delete data;
}

void Waterfall::wheelEvent(int delta){
    if(spectrum_view->paused || !active){   // Waterfall needs to be paused if EyeThread is still running or if it stopped scrolling can still be allowed
      if(delta > 0) spectrum_view->scroll_up();
      else spectrum_view->scroll_down();
    }
}

void Waterfall::SubWindow_mousePressEvent(){
  if(active){
    spectrum_view->paused = !spectrum_view->paused;
    spectrum_view->scroll_offset = 0;
  }
}

void Waterfall::setFPS(int fps){
  // fps is in 1/s --> we need msec!
  glob_settings->glob_args.gui_args.wf_fps = fps;
  glob_settings->store_settings();
}
// Waterfall_UL

Waterfall_UL::Waterfall_UL(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea):
  Waterfall(p_glob_settings, p_spectrumAdapter, p_mdiArea){
  this->decorate();
}

Waterfall_UL::~Waterfall_UL(){
  // Just deactivate the sub class here,
  // base classes destructor will be called automatically afterwards!
  this->deactivate();
}

void Waterfall_UL::activate(){
  active = true;

  // Connect signals/slots
  connect (spectrumAdapter, SIGNAL(update_ul(const ScanLineLegacy*)),SLOT(draw(const ScanLineLegacy*)));
  connect (spectrum_view, SIGNAL(subwindow_click()) ,SLOT(SubWindow_mousePressEvent()));
  spectrumAdapter->emit_uplink = true;

  spectrum_view->paused = false;
}

void Waterfall_UL::deactivate(){
  active = false;

  disconnect(spectrumAdapter, SIGNAL(update_ul(const ScanLineLegacy*)),this,SLOT(draw(const ScanLineLegacy*)));
  spectrum_view->disconnect();
}

void Waterfall_UL::decorate(){
  // Set properties
  m_window->setObjectName("Uplink");
  m_window->setWindowTitle("Uplink RB Allocations");
  spectrum_view->setObjectName("Spectrum View UL");
}

// Waterfall_DL

Waterfall_DL::Waterfall_DL(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea):
  Waterfall(p_glob_settings, p_spectrumAdapter, p_mdiArea){
  this->decorate();
}

Waterfall_DL::~Waterfall_DL(){
  // Just deactivate the sub class here,
  // base classes destructor will be called automatically afterwards!
  this->deactivate();
}

void Waterfall_DL::activate(){
  active = true;

  // Connect signals/slots
  connect (spectrumAdapter, SIGNAL(update_dl(const ScanLineLegacy*)),SLOT(draw(const ScanLineLegacy*)));
  connect (spectrum_view, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
  spectrumAdapter->emit_downlink = true;

  spectrum_view->paused = false;
}

void Waterfall_DL::deactivate(){
  active = false;

  disconnect(spectrumAdapter, SIGNAL(update_dl(const ScanLineLegacy*)),this,SLOT(draw(const ScanLineLegacy*)));
  spectrum_view->disconnect();
}

void Waterfall_DL::decorate(){
  // Set properties
  m_window->setObjectName("Downlink");
  m_window->setWindowTitle("Downlink RB Allocations");
  spectrum_view->setObjectName("Spectrum View DL");
}

// Waterfall_DIFF

Waterfall_DIFF::Waterfall_DIFF(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea):
  Waterfall(p_glob_settings, p_spectrumAdapter, p_mdiArea){
  this->decorate();
}

Waterfall_DIFF::~Waterfall_DIFF(){
  // Just deactivate the sub class here,
  // base classes destructor will be called automatically afterwards!
  this->deactivate();
}

void Waterfall_DIFF::activate(){
  active = true;

  // Connect signals/slots
  connect (spectrumAdapter, SIGNAL(update_spectrum_diff(const ScanLineLegacy*)),SLOT(draw(const ScanLineLegacy*)));
  connect (spectrum_view, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
  spectrumAdapter->emit_difference = true;

  spectrum_view->paused = false;
}

void Waterfall_DIFF::deactivate(){
  active = false;

  disconnect(spectrumAdapter, SIGNAL(update_spectrum_diff(const ScanLineLegacy*)),this,SLOT(draw(const ScanLineLegacy*)));
  spectrum_view->disconnect();
}

void Waterfall_DIFF::decorate(){
  // Set properties
  m_window->setObjectName("Spectrum Diff");
  m_window->setWindowTitle("Spectrum Difference");
  spectrum_view->setObjectName("Spectrum View Diff");
}

// Waterfall_SPEC

Waterfall_SPEC::Waterfall_SPEC(Settings* p_glob_settings, SpectrumAdapter* p_spectrumAdapter, QMdiArea* p_mdiArea):
  Waterfall(p_glob_settings, p_spectrumAdapter, p_mdiArea){
  this->decorate();
}

Waterfall_SPEC::~Waterfall_SPEC(){
  // Just deactivate the sub class here,
  // base classes destructor will be called automatically afterwards!
  this->deactivate();
}

void Waterfall_SPEC::activate(){
  active = true;

  // Connect signals/slots
  connect (spectrumAdapter, SIGNAL(update_spectrum(const ScanLineLegacy*)),SLOT(draw(const ScanLineLegacy*)));
  connect (spectrum_view, SIGNAL(subwindow_click()),SLOT(SubWindow_mousePressEvent()));
  spectrumAdapter->emit_spectrum = true;

  spectrum_view->paused = false;
}

void Waterfall_SPEC::deactivate(){
  active = false;

  disconnect(spectrumAdapter, SIGNAL(update_spectrum(const ScanLineLegacy*)),this,SLOT(draw(const ScanLineLegacy*)));
  spectrum_view->disconnect();
}

void Waterfall_SPEC::decorate(){
  // Set properties
  m_window->setObjectName("Spectrum");
  m_window->setWindowTitle("Downlink Spectrum");
  spectrum_view->setObjectName("Spectrum View");
}
void Waterfall_SPEC::range_slider_value_changed(int _1st, int _2nd){
  if(_1st > _2nd){
    spectrum_view->max_intensity = _1st;
    spectrum_view->min_intensity = _2nd;
  }else{
    spectrum_view->max_intensity = _2nd;
    spectrum_view->min_intensity = _1st;
  }
  spectrum_view->intensity_factor = (1.125 * (float)USHRT_MAX) / (spectrum_view->max_intensity - spectrum_view->min_intensity);   // Calculate Intensity factor for dynamic spectrum
}
