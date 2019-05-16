#include "cni_cc_decoderThread.h"
#include "cni_cc_decoder.h"
#include "ScanThread.h"
//#include "src/gui/plot_data.h"

#include <unistd.h>
#include <iostream>



void DecoderThread::init() {
  // setup dependencies of this instance
  initialized = true;
}

void DecoderThread::start() {
  if(!isInitialized()) {
    std::cerr << "Error in " << __func__ << ": not initialized." << std::endl;
    return;
  }
  if(theThread != NULL) {
    std::cerr << "Thread already running!" << std::endl;
  }

  decoderthread = this;
  go_exit = false;

  //setvariables;

  theThread = new boost::thread(boost::bind(&DecoderThread::run, this));
}

void DecoderThread::run() {


  start_cni_decoder();



}

void DecoderThread::stop() {
  go_exit = true;
  if(theThread != NULL) {
    theThread->join();
    delete theThread;
    theThread = NULL;
  }
}

bool DecoderThread::isInitialized() {
  return initialized;
}

DecoderThread::~DecoderThread() {
  stop();
}


extern "C" void call_function(void *f_pointer, float *data_ul, float *data_dl, float *data_spectrum, uint32_t *rnti_hist){

  DecoderThread* inst = static_cast<DecoderThread*>(f_pointer);
  if(inst != NULL) {

    ScanLineLegacy *scanline_spectrum = new ScanLineLegacy;
    ScanLineLegacy *scanline_spectrum_diff = new ScanLineLegacy;
    ScanLineLegacy *scanline_ul = new ScanLineLegacy;
    ScanLineLegacy *scanline_dl = new ScanLineLegacy;
    ScanLineLegacy *scanline_rnti_hist = new ScanLineLegacy;

    scanline_rnti_hist->type = SCAN_LINE_RNTI_HIST;
    scanline_ul->type = SCAN_LINE_UPLINK;
    scanline_dl->type = SCAN_LINE_DOWNLINK;
    scanline_spectrum->type  = SCAN_LINE_SPECTRUM;
    scanline_spectrum_diff->type = SCAN_LINE_SPECTRUM_DIFF;

    //ScanLine Data:

    for(int i = 0; i < SPECTROGRAM_LINE_WIDTH; i++)
    {
      scanline_ul->linebuf[i] = data_ul[i];
      scanline_dl->linebuf[i] = data_dl[i];
      scanline_spectrum->linebuf[i] = data_spectrum[i];

      //If data is decoded, filter them out

      if(scanline_dl->linebuf[i] == 0){
        if(data_spectrum[i] >= 25000) scanline_spectrum_diff->linebuf[i] = 25000;
        else scanline_spectrum_diff->linebuf[i] = data_spectrum[i];
      }
      else{
        scanline_spectrum_diff->linebuf[i] = 45000; // Bright Color for better visibility
        if(scanline_spectrum->linebuf[i] <= 2000) scanline_spectrum_diff->linebuf[i] = 62000; // Detecting false Positive
      }
    }

    //RNTI Data:

    for(int i = 0; i < 65536; i++){
      scanline_rnti_hist->rnti_hist[i] = rnti_hist[i];
    }

    inst->pushToSubscribers(scanline_ul);
    inst->pushToSubscribers(scanline_dl);
    inst->pushToSubscribers(scanline_spectrum);
    inst->pushToSubscribers(scanline_spectrum_diff);
    inst->pushToSubscribers(scanline_rnti_hist);

  }

}

extern "C" void call_function_2(void* goal, uint16_t sfn, uint32_t sf_idx,uint32_t mcs_idx,int mcs_tbs,uint32_t l_prb){

  DecoderThread* inst = static_cast<DecoderThread*>(goal);
  if(inst != NULL) {

    ScanLineLegacy *scanline_perf_plots = new ScanLineLegacy;

    scanline_perf_plots->type = SCAN_LINE_PERF_PLOT_A;

    scanline_perf_plots->sf_idx   = sf_idx;
    scanline_perf_plots->sfn      = sfn;
    scanline_perf_plots->mcs_tbs  = mcs_tbs;
    scanline_perf_plots->mcs_idx  = mcs_idx;
    scanline_perf_plots->l_prb    = l_prb;

    inst->pushToSubscribers(scanline_perf_plots);

  }
}

extern "C" void call_function_3(void* goal, uint16_t sfn, uint32_t sf_idx,uint32_t mcs_idx,int mcs_tbs,uint32_t l_prb){

  DecoderThread* inst = static_cast<DecoderThread*>(goal);
  if(inst != NULL) {

    ScanLineLegacy *scanline_perf_plots = new ScanLineLegacy;

    scanline_perf_plots->type = SCAN_LINE_PERF_PLOT_B;

    scanline_perf_plots->sf_idx   = sf_idx;
    scanline_perf_plots->sfn      = sfn;
    scanline_perf_plots->mcs_tbs  = mcs_tbs;
    scanline_perf_plots->mcs_idx  = mcs_idx;
    scanline_perf_plots->l_prb    = l_prb;

    inst->pushToSubscribers(scanline_perf_plots);

  }
}
