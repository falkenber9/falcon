#include "EyeThread.h"

#include <unistd.h>
#include <iostream>

#include "eye/EyeCore.h"

void plot_scanLines(EyeThread *inst, const std::vector<uint16_t> *data_ul, const std::vector<uint16_t> *data_dl, const std::vector<float> *data_spectrum){

  if (dynamic_cast<EyeThread*>(inst) == nullptr){
    ERROR("Handed wrong object type to function >>\tplot_scanlines\t<<!");
  }
  else{
    ScanLineLegacy *scanline_spectrum = new ScanLineLegacy;
    ScanLineLegacy *scanline_spectrum_diff = new ScanLineLegacy;
    ScanLineLegacy *scanline_ul = new ScanLineLegacy;
    ScanLineLegacy *scanline_dl = new ScanLineLegacy;

    scanline_ul->type = SCAN_LINE_UPLINK;
    scanline_ul->l_prb = data_ul->size();


    scanline_dl->type = SCAN_LINE_DOWNLINK;
    scanline_dl->l_prb = data_dl->size();

    scanline_spectrum->type  = SCAN_LINE_SPECTRUM;
    scanline_spectrum->l_prb = data_spectrum->size();

    scanline_spectrum_diff->type = SCAN_LINE_SPECTRUM_DIFF;
    scanline_spectrum_diff->l_prb = data_spectrum->size();

    //ScanLine Data:
    for(int i = 0; i < data_dl->size(); i++)
    {
      // Casting to float for legacy reasons (should be done better for performance!)
      scanline_ul->linebuf[i] = data_ul->at(i);
      scanline_dl->linebuf[i] = data_dl->at(i);
      scanline_spectrum->linebuf[i] = data_spectrum->at(i);

      //If data is decoded, filter them out

      if(scanline_dl->linebuf[i] == 0){
        if(data_spectrum->at(i) >= 25000) {
          scanline_spectrum_diff->linebuf[i] = 25000;
        }
        else {
          scanline_spectrum_diff->linebuf[i] = data_spectrum->at(i);
        }
      }
      else {
        scanline_spectrum_diff->linebuf[i] = 45000; // Bright Color for better visibility
        if(scanline_spectrum->linebuf[i] <= 2000) {
          scanline_spectrum_diff->linebuf[i] = 62000; // Detecting false Positive
        }
      }
    }


    inst->pushToSubscribers(scanline_ul);
    inst->pushToSubscribers(scanline_dl);
    inst->pushToSubscribers(scanline_spectrum);
    inst->pushToSubscribers(scanline_spectrum_diff);
  }

}

void update_perfPlot(EyeThread* inst, uint32_t sfn, uint32_t sf_idx,uint32_t mcs_idx,int mcs_tbs,uint32_t l_prb, ScanLineType_t sl_type, uint32_t total_prb){

  if (dynamic_cast<EyeThread*>(inst) == nullptr){
    ERROR("Handed wrong object type to function >>\tupdate_perfPlot\t<<!");
  }
  else{

    ScanLineLegacy *scanline_perf_plots = new ScanLineLegacy;

    // If sl_type is neither SCAN_LINE_PERF_PLOT_A or SCAN_LINE_PERF_PLOT_B --> call ERROR
    if(!((sl_type == SCAN_LINE_PERF_PLOT_A) || (sl_type == SCAN_LINE_PERF_PLOT_B))){
      ERROR("Wrong scanline type submitted!");
    }
    else{
      scanline_perf_plots->type     = sl_type;
      scanline_perf_plots->sf_idx   = sf_idx;
      scanline_perf_plots->sfn      = sfn;
      scanline_perf_plots->mcs_tbs  = mcs_tbs;
      scanline_perf_plots->mcs_idx  = mcs_idx;
      scanline_perf_plots->l_prb    = l_prb;
      scanline_perf_plots->total_prb = total_prb;

      inst->pushToSubscribers(scanline_perf_plots);
    }
  }
}

void plot_rnti(EyeThread *inst, uint32_t *rnti_hist){
  if (dynamic_cast<EyeThread*>(inst) == nullptr){
    ERROR("Handed wrong object type to function >>\tupdate_perfPlot\t<<!");
  }
  else{
    ScanLineLegacy *scanline_rnti = new ScanLineLegacy;
    scanline_rnti->type = SCAN_LINE_RNTI_HIST;
    scanline_rnti->rnti_active_set = inst->getRNTIManager().getActiveSet();
    
    inst-> pushToSubscribers(scanline_rnti);
  }
}

void update_tableEntry(EyeThread *inst, uint32_t sfn, uint32_t sf_idx,uint32_t mcs_idx,int mcs_tbs,uint32_t l_prb, uint16_t rnti){
  if (dynamic_cast<EyeThread*>(inst) == nullptr){
    ERROR("Handed wrong object type to function >>\tupdate_tableEntry\t<<!");
  }
  else{
    ScanLineLegacy *tableLine = new ScanLineLegacy;
    tableLine->type = TABLE_LINE;
    tableLine->sf_idx   = sf_idx;
    tableLine->sfn      = sfn;
    tableLine->mcs_tbs  = mcs_tbs;
    tableLine->mcs_idx  = mcs_idx;
    tableLine->l_prb    = l_prb;
    tableLine->rnti = rnti;

    inst-> pushToSubscribers(tableLine);
  }
}

void EyeThread::init() {
  // setup dependencies of this instance
  initialized = true;

}

void EyeThread::start(const Args& args) {
  if(!isInitialized()) {
    std::cerr << "Error in " << __func__ << ": not initialized." << std::endl;
    return;
  }

  if(theThread != nullptr) {
    std::cerr << "Thread already running!" << std::endl;
    return;
  }

  if(eye != nullptr) {
    delete eye;
    eye = nullptr;
  }

  eye = new EyeCore(args);
  eye->setDCIConsumer(m_consumer);
  theThread = new boost::thread(boost::bind(&EyeThread::run, this));
}

void EyeThread::run() {
  bool success = eye->run();
  (void)success;
}

void EyeThread::stop() {
  if(eye != nullptr) {
    eye->stop();
    if(theThread != nullptr) {
      theThread->join();
      delete theThread;
      theThread = nullptr;
    }
    delete eye;
    eye = nullptr;
  }
}

bool EyeThread::isInitialized() {
  return initialized;
}

void EyeThread::refreshShortcutDiscovery(bool val){
  if(eye){eye->refreshShortcutDiscovery(val);}
}

void EyeThread::forwardRNTIHistogramThresholdToEyeCore(int val){
  if(eye){eye->setRNTIThreshold(val);}
}


EyeThread::~EyeThread() {
  stop();
}

void EyeThread::attachConsumer(std::shared_ptr<SubframeInfoConsumer> consumer){
  m_consumer = consumer;
}

RNTIManager &EyeThread::getRNTIManager(){
  return eye->getRNTIManager();
}

DCIGUIConsumer::DCIGUIConsumer(){}
DCIGUIConsumer::DCIGUIConsumer(EyeThread &p_Thread){
  this->setThread(p_Thread);
}


void DCIGUIConsumer::setThread(EyeThread &p_Thread){
  if(&p_Thread != nullptr){
    m_Thread = &p_Thread;
  }
}

void DCIGUIConsumer::consumeDCICollection(const SubframeInfo& subframeInfo) {
  uint32_t nof_prb = subframeInfo.getSubframePower().getNofPRB();
  const DCICollection& collection(subframeInfo.getDCICollection());
  // Required as reference!!

  // Power spectrum normalization to fit to colormap
  const std::vector<float>& power(subframeInfo.getSubframePower().getRBPowerDL());
  std::vector<float> spectrumPixmap(power.size());
  //normalize... expected values are -10...20
  //normalize to 0...65535
  for (uint32_t prb = 0; prb < power.size(); prb++) {
    spectrumPixmap[prb] = (power[prb] + 10) * 2000;
  }

  std::vector<uint16_t> cmRBMapUL = collection.applyLegacyColorMap(collection.getRBMapUL());
  std::vector<uint16_t> cmRBMapDL = collection.applyLegacyColorMap(collection.getRBMapDL());

  plot_scanLines(m_Thread,
                 &cmRBMapUL,
                 &cmRBMapDL,
                 &spectrumPixmap
                 );

  plot_rnti(m_Thread, nullptr);

  // Downlink
  const std::vector<DCI_DL>& dci_dl = collection.getDCI_DL();
  for(std::vector<DCI_DL>::const_iterator dci_dl_it = dci_dl.begin(); dci_dl_it != dci_dl.end(); ++dci_dl_it) {
    int tb_count = 0;
    int tbs_sum = 0;
    uint32_t idx_avg = 0;
    switch(dci_dl_it->format) {
      case SRSLTE_DCI_FORMAT0:
        ERROR("Error: no reason to be here\n");
        break;
      case SRSLTE_DCI_FORMAT1:
      case SRSLTE_DCI_FORMAT1A:
      case SRSLTE_DCI_FORMAT1C:
      case SRSLTE_DCI_FORMAT1B:
      case SRSLTE_DCI_FORMAT1D:
        update_perfPlot  (m_Thread,
                          collection.get_sfn(),
                          collection.get_sf_idx(),
                          dci_dl_it->dl_grant->mcs[0].idx,
                          dci_dl_it->dl_grant->mcs[0].tbs,
                          dci_dl_it->dl_grant->nof_prb,
                          SCAN_LINE_PERF_PLOT_B,
                          nof_prb);
        update_tableEntry(m_Thread,
                          collection.get_sfn(),
                          collection.get_sf_idx(),
                          dci_dl_it->dl_grant->mcs[0].idx,
                          dci_dl_it->dl_grant->mcs[0].tbs,
                          dci_dl_it->dl_grant->nof_prb,
                          dci_dl_it->rnti);
        break;
      case SRSLTE_DCI_FORMAT2:
      case SRSLTE_DCI_FORMAT2A:
      case SRSLTE_DCI_FORMAT2B:
#define DCI_FORMAT2X_NOF_TB 2
        for(int tb = 0; tb < DCI_FORMAT2X_NOF_TB; tb++) {
          if(dci_dl_it->dl_grant->tb_en[tb]) {
            tb_count++;
            tbs_sum += dci_dl_it->dl_grant->mcs[tb].tbs;
            idx_avg += dci_dl_it->dl_grant->mcs[tb].idx;
          }
        }
        if(tb_count > 0) {
          idx_avg /= tb_count;
        }
        update_perfPlot  (m_Thread,
                          collection.get_sfn(),
                          collection.get_sf_idx(),
                          idx_avg,
                          tbs_sum,
                          dci_dl_it->dl_grant->nof_prb,
                          SCAN_LINE_PERF_PLOT_B, nof_prb);
        update_tableEntry(m_Thread,
                          collection.get_sfn(),
                          collection.get_sf_idx(),
                          idx_avg,
                          tbs_sum,
                          dci_dl_it->dl_grant->nof_prb,
                          dci_dl_it->rnti);
        break;
        //case SRSLTE_DCI_FORMAT3:
        //case SRSLTE_DCI_FORMAT3A:
      default:
        ERROR("Other formats\n");
    }
  }

  // Uplink
  const std::vector<DCI_UL>& dci_ul = collection.getDCI_UL();
  for(std::vector<DCI_UL>::const_iterator dci_ul_it = dci_ul.begin(); dci_ul_it != dci_ul.end(); ++dci_ul_it) {
    if (dci_ul_it->ul_dci_unpacked->mcs_idx < 29) { // Regular MCS
      update_perfPlot  (m_Thread,
                        collection.get_sfn(),
                        collection.get_sf_idx(),
                        dci_ul_it->ul_grant->mcs.idx,
                        dci_ul_it->ul_grant->mcs.tbs,
                        dci_ul_it->ul_grant->L_prb,
                        SCAN_LINE_PERF_PLOT_A,
                        nof_prb);
      update_tableEntry(m_Thread,
                        collection.get_sfn(),
                        collection.get_sf_idx(),
                        dci_ul_it->ul_grant->mcs.idx,
                        dci_ul_it->ul_grant->mcs.tbs,
                        dci_ul_it->ul_grant->L_prb,
                        dci_ul_it->rnti);
    }
    else {  // MCS >= 29 are reserved for special cases, such as retransmissions
      update_perfPlot  (m_Thread,
                        collection.get_sfn(),
                        collection.get_sf_idx(),
                        dci_ul_it->ul_grant->mcs.idx,
                        0,
                        dci_ul_it->ul_grant->L_prb,
                        SCAN_LINE_PERF_PLOT_A,
                        nof_prb);
      update_tableEntry(m_Thread,
                        collection.get_sfn(),
                        collection.get_sf_idx(),
                        dci_ul_it->ul_grant->mcs.idx,
                        0,
                        dci_ul_it->ul_grant->L_prb,
                        dci_ul_it->rnti);
    }
  }
}

