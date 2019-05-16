#include "CaptureProbeCore.h"
#include "falcon/common/SubframeBuffer.h"
#include "falcon/phy/falcon_rf/rf_imp.h"

#include <iostream>
#include <fstream>
#include <signal.h>
#include <unistd.h>

using namespace std;

static cell_search_cfg_t cell_detect_config = {
  SRSLTE_DEFAULT_MAX_FRAMES_PBCH,
  SRSLTE_DEFAULT_MAX_FRAMES_PSS,
  SRSLTE_DEFAULT_NOF_VALID_PSS_FRAMES,
  0
};

bool isZero(double value) {
  const double epsilon = 1e-5;
  return (value < epsilon) && (value > -epsilon);
}

//int falcon_rf_recv_wrapper(void *h, cf_t *data[SRSLTE_MAX_PORTS], uint32_t nsamples, srslte_timestamp_t *t) {
//  (void)t;  //unused
//  DEBUG(" ----  Receive %d samples  ---- \n", nsamples);
//  void *ptr[SRSLTE_MAX_PORTS];
//  for (int i=0;i<SRSLTE_MAX_PORTS;i++) {
//    ptr[i] = data[i];
//  }
//  return srslte_rf_recv_with_time_multi(static_cast<srslte_rf_t*>(h), ptr, nsamples, true, nullptr, nullptr);
//}

//double falcon_rf_set_rx_gain_th_wrapper(void *h, double f) {
//  return srslte_rf_set_rx_gain_th(static_cast<srslte_rf_t*>(h), f);
//}

CaptureProbeCore::CaptureProbeCore(Args& args) :
  SignalHandler(),
  netsync(nullptr),
  modem(nullptr),
  gps(nullptr),
  trafficGen(nullptr),
  sink(nullptr),
  go_exit(false),
  critical(false),
  args(args)
{
  for (int i = 0; i< SRSLTE_MAX_CODEWORDS; i++) {
    data[i] = static_cast<uint8_t*>(srslte_vec_malloc(sizeof(uint8_t)*1500*8));
    if (!data[i]) {
      cout << "Error allocating buffer for sample data" << endl;
      critical = true;
      go_exit = true;
    }
  }
}

CaptureProbeCore::~CaptureProbeCore() {
  for (int i = 0; i< SRSLTE_MAX_CODEWORDS; i++) {
    free(data[i]);
  }
}

void CaptureProbeCore::init(NetsyncSlave* netsync,
                            AuxModem* modem,
                            TrafficGenerator* trafficGen,
                            FileSink<cf_t>* sink, GPS* gps) {
  go_exit = false;

  this->netsync = netsync;
  netsync->attachCancelable(this);
  this->modem = modem;
  this->trafficGen = trafficGen;
  this->sink = sink;
  this->gps = gps;

}

void CaptureProbeCore::cancel() {
  go_exit = true;
}

bool CaptureProbeCore::run() {
  int n, ret;
  int decimate = 1;
  srslte_cell_t cell;
  int64_t sf_cnt, sf_guard;
  srslte_ue_dl_t ue_dl;
  srslte_ue_sync_t ue_sync;
  srslte_ue_mib_t ue_mib;
  srslte_rf_t rf;

  GPSFix gpsFix;

  uint8_t bch_payload[SRSLTE_BCH_PAYLOAD_LEN];
  int32_t sfn_offset = 0;
  bool fstart = 0;
  float cfo = 0;

  //cf_t *sf_buffer[SRSLTE_MAX_PORTS] = {nullptr};
  SubframeBuffer sfb(args.rf_nof_rx_ant);
  uint32_t sfn = 0; // system frame number

  if(args.client_mode) {
    if(netsync->waitStart()) {
      const NetsyncMessageStart& remoteParams(netsync->getRemoteParams());
      args.output_file_base_name = remoteParams.getId();
      args.nof_subframes = remoteParams.getNofSubframes();
      args.probing_delay = remoteParams.getOffsetSubframes();
      args.direction = remoteParams.getDirection();
      args.payload_size = remoteParams.getPayloadSize();
      args.url = remoteParams.getUrl();
      gpsFix.latitude = remoteParams.getLatitude();
      gpsFix.longitude = remoteParams.getLongitude();
      args.tx_power_sample_interval = remoteParams.getTxPowerSamplingInterval();
    }
    else {
      // canceled by CTRL+C signal or stop message
      return false;
    }
  }
  else {
    if(gps != nullptr) {
      gpsFix = gps->getFix();
    }
  }

  if(args.backoff > 0) {
    uint32_t backoff = (args.backoff * (args.probing_delay + DEFAULT_PROBING_TIMEOUT_MS)) / 1000;
    *netsync << "Backoff for " << backoff << "s" << endl;
    sleep(backoff);
  }

  if(!modem->setOnline(true)) {
    *netsync << "Could not establish data connection via aux modem" << endl;
    return true;
  }
  *netsync << "Established data connection" << endl;

  if(args.tx_power_sample_interval > 0) {
    modem->configureTXPowerSampling(args.tx_power_sample_interval);
  }

  string operatorName = modem->getOperatorName();
  string fileSinkFileName(args.output_file_base_name + "-" + operatorName + "-iq.bin");
  string trafficResultsFileName(args.output_file_base_name + "-" + operatorName + "-traffic.csv");
  string cellInfoFileName(args.output_file_base_name + "-" + operatorName + "-cell.csv");
  string txPowerSamplesFileName(args.output_file_base_name + "-" + operatorName + "-txpower.csv");

  //TrafficResultsToFileAndNetsyncMessages* trafficResultsHandler = new TrafficResultsToFileAndNetsyncMessages(trafficGen, trafficResultsFileName, netsync);
  TrafficResultsToFileAndNetsyncMessagesStopTxPowerSampling* trafficResultsHandler = new TrafficResultsToFileAndNetsyncMessagesStopTxPowerSampling(trafficGen, trafficResultsFileName, netsync, modem);
  trafficResultsHandler->setOutputFileName(trafficResultsFileName);
  trafficGen->setEventHandler(trafficResultsHandler); // takes ownership
  trafficResultsHandler = nullptr;

  sink->open(fileSinkFileName);

  if(args.cpu_affinity > -1) {
    cpu_set_t cpuset;
    pthread_t thread;

    thread = pthread_self();
    for(int i = 0; i < 8;i++){
      if(((args.cpu_affinity >> i) & 0x01) == 1){
        *netsync << "Setting cni_capture_sync with affinity to core " << i << endl;
        CPU_SET(static_cast<size_t>(i), &cpuset);
      }
      if(pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset)){
        *netsync << "Error setting main thread affinity to " << args.cpu_affinity << endl;
        return true;
      }
    }
  }
  *netsync << "Opening RF device with " << args.rf_nof_rx_ant <<
              " RX antennas..." << endl;
  char rfArgsCStr[1024];  /* WTF! srslte_rf_open_multi takes char*, not const char* ! */
  strncpy(rfArgsCStr, args.rf_args.c_str(), 1024);
  if (srslte_rf_open_multi(&rf, rfArgsCStr, args.rf_nof_rx_ant)) {
    *netsync <<  "Error opening rf" << endl;
    return true;
  }
  /* Set receiver gain */
  if (args.rf_gain > 0) {
    srslte_rf_set_rx_gain(&rf, args.rf_gain);
  }
  else {
    *netsync << "Starting AGC thread..." << endl;
    if (srslte_rf_start_gain_thread(&rf, false)) {
      *netsync << "Error starting AGC thread" << endl;
      return true;
    }
    srslte_rf_set_rx_gain(&rf, srslte_rf_get_rx_gain(&rf));
    cell_detect_config.init_agc = static_cast<float>(srslte_rf_get_rx_gain(&rf));
  }

  srslte_rf_set_master_clock_rate(&rf, 30.72e6);

  NetworkInfo* netinfoBefore = nullptr;
  NetworkInfo* netinfoProbingStart = nullptr;
  if(!args.no_auxmodem) {
    /* read network information from modem */
    //NetworkInfo netinfoBefore(modem->getNetworkInfo());
    netinfoBefore = new NetworkInfo(modem->getNetworkInfo());
    if(!netinfoBefore->isValid()) {
      *netsync << "Error: could not get network info from aux modem" << endl;
      srslte_rf_kill_gain_thread(&rf);
      srslte_rf_close(&rf);
      delete netinfoBefore;
      return true;
    }
  }

  /* set receiver frequency */
  double rf_freq;
  int N_id_2;
  if(isZero(args.rf_freq) && !args.no_auxmodem) {
    rf_freq = netinfoBefore->rf_freq * 1e6;
    N_id_2 = netinfoBefore->N_id_2;
  }
  else {
    rf_freq = args.rf_freq;
    N_id_2 = -1;
    *netsync << "Receive frequency override to " << rf_freq << " Hz" << endl;
  }
  *netsync << "Tunning receiver to " << rf_freq << " Hz" << endl;
  srslte_rf_set_rx_freq(&rf, rf_freq);
  srslte_rf_rx_wait_lo_locked(&rf);

  uint32_t ntrial = 0;
  uint32_t max_trial = 3;
  do {
    ret = rf_search_and_decode_mib(&rf, args.rf_nof_rx_ant, &cell_detect_config, N_id_2, &cell, &cfo);
    if (ret < 0) {
      *netsync << "Error searching for cell" << endl;
      go_exit = true;
    } else if (ret == 0 && !go_exit) {
      *netsync << "Cell not found after " << ntrial++ << " trials" << endl;
    }
    if (ntrial >= max_trial) go_exit = true;
  } while (ret == 0 && !go_exit);

  srslte_rf_stop_rx_stream(&rf);
  srslte_rf_flush_buffer(&rf);

  if (go_exit) {
    srslte_rf_kill_gain_thread(&rf);
    srslte_rf_close(&rf);
    delete netinfoBefore;
    return true;
  }

#ifdef DISABLED__
  srslte_rf_stop_rx_stream(&rf);
  srslte_rf_flush_buffer(&rf);
#endif

  /* set sampling frequency */
  int srate = srslte_sampling_freq_hz(cell.nof_prb);
  if (srate != -1) {
    if (srate < 10e6) {
      srslte_rf_set_master_clock_rate(&rf, 4*srate);
    } else {
      srslte_rf_set_master_clock_rate(&rf, srate);
    }
    *netsync << "Setting sampling rate " << (srate)/1000000 << " MHz" << endl;
    double srate_rf = srslte_rf_set_rx_srate(&rf, static_cast<double>(srate));
    if (static_cast<int>(srate_rf) != srate) {
      *netsync << "Could not set sampling rate" << endl;
      return true;
    }
  } else {
    *netsync << "Invalid number of PRB " << cell.nof_prb << endl;
    return true;
  }

  *netsync << "Stopping RF and flushing buffer..." << endl;

  if(args.decimate) {
    if(args.decimate > 4 || args.decimate < 0) {
      *netsync << "Invalid decimation factor, setting to 1" << endl;
    }
    else {
      decimate = args.decimate;
      //ue_sync.decimate = prog_args.decimate;
    }
  }
  if (srslte_ue_sync_init_multi_decim(&ue_sync,
                                      cell.nof_prb,
                                      cell.id==1000,
                                      falcon_rf_recv_wrapper,
                                      args.rf_nof_rx_ant,
                                      static_cast<void*>(&rf), decimate)) {
    *netsync << "Error initiating ue_sync" << endl;
    return true;
  }

  if (srslte_ue_sync_set_cell(&ue_sync, cell)) {
    *netsync << "Error initiating ue_sync" << endl;
    return true;
  }

//  for (uint32_t i=0;i<args.rf_nof_rx_ant;i++) {
//    sf_buffer[i] = static_cast<cf_t*>(srslte_vec_malloc(3*static_cast<uint32_t>(sizeof(cf_t))*static_cast<uint32_t>(SRSLTE_SF_LEN_PRB(cell.nof_prb))));
//  }

  if (srslte_ue_mib_init(&ue_mib, sfb.sf_buffer, cell.nof_prb)) {
    *netsync << "Error initaiting UE MIB decoder" << endl;
    return true;
  }
  if (srslte_ue_mib_set_cell(&ue_mib, cell)) {
    *netsync << "Error initaiting UE MIB decoder" << endl;
    return true;
  }

  if (srslte_ue_dl_init(&ue_dl, sfb.sf_buffer, cell.nof_prb, args.rf_nof_rx_ant)) {
    //if (srslte_ue_dl_init(&ue_dl, sf_buffer, cell.nof_prb, prog_args.rf_nof_rx_ant)) {
    *netsync << "Error initiating UE downlink processing module" << endl;
    return true;
  }
  if (srslte_ue_dl_set_cell(&ue_dl, cell)) {
    *netsync << "Error initiating UE downlink processing module" << endl;
    return true;
  }

  // Disable CP based CFO estimation during find
  ue_sync.cfo_current_value = cfo/15000;
  ue_sync.cfo_is_copied = true;
  ue_sync.cfo_correct_enable_find = true;
  srslte_sync_set_cfo_cp_enable(&ue_sync.sfind, false, 0);


  srslte_chest_dl_cfo_estimate_enable(&ue_dl.chest, false, 1023); // args->enable_cfo_ref = false;
  srslte_chest_dl_average_subframe(&ue_dl.chest, false);          // args->average_subframe = false;

  /* Initialize subframe counter */
  sf_cnt = 0;
  sf_guard = 0;

  srslte_rf_start_rx_stream(&rf, false);

  if (args.rf_gain < 0) {
    srslte_rf_info_t *rf_info = srslte_rf_get_info(&rf);
    srslte_ue_sync_start_agc(&ue_sync,
                             falcon_rf_set_rx_gain_th_wrapper,
                             rf_info->min_rx_gain,
                             rf_info->max_rx_gain,
                             static_cast<double>(cell_detect_config.init_agc));
  }

#ifdef PRINT_CHANGE_SCHEDULING
  srslte_ra_dl_dci_t old_dl_dci;
  bzero(&old_dl_dci, sizeof(srslte_ra_dl_dci_t));
#endif

  ue_sync.cfo_correct_enable_track = !args.disable_cfo;

  srslte_pbch_decode_reset(&ue_mib.pbch);

  // in case of no physical auxmodem, provide cell info to the dummy and refresh netinfoBefore
  modem->inform(cell.nof_prb, static_cast<int>(cell.id), rf_freq);
  if(args.no_auxmodem) {
    delete netinfoBefore; // actually, netinfoBefore should always be nullptr here anyway
    netinfoBefore = new NetworkInfo(modem->getNetworkInfo());
  }
  // in case of no probing, clone netinfoBefore into netinfoProbingStart
  netinfoProbingStart = new NetworkInfo(*netinfoBefore);

  *netsync << "Entering main loop..." << endl;
  /* Main loop */
  while (!go_exit && (sf_cnt < args.nof_subframes || args.nof_subframes == 0)) {

    ret = srslte_ue_sync_zerocopy_multi(&ue_sync, sfb.sf_buffer);
    if (ret < 0) {
      *netsync << "Error calling srslte_ue_sync_work()" << endl;
    }

#ifdef CORRECT_SAMPLE_OFFSET
    float sample_offset = (float) srslte_ue_sync_get_last_sample_offset(&ue_sync)+srslte_ue_sync_get_sfo(&ue_sync)/1000;
    srslte_ue_dl_set_sample_offset(&ue_dl, sample_offset);
#endif

    /* srslte_ue_sync_zerocopy_multi returns 1 if successfully read 1 aligned subframe */
    if (ret == 1) {
      if (srslte_ue_sync_get_sfidx(&ue_sync) == 0) {
        n = srslte_ue_mib_decode(&ue_mib, bch_payload, nullptr, &sfn_offset);
        if (n < 0) {
          *netsync << "Error decoding UE MIB" << endl;
          go_exit = true;
        } else if (n == SRSLTE_UE_MIB_FOUND) {
          srslte_pbch_mib_unpack(bch_payload, &cell, &sfn);
          cout << "Decoded MIB. SFN: " << sfn <<
                  ", offset " << sfn_offset << endl;
          if (cell.nof_prb <= 100) {  /* skip occasional nof_prb=150 in bad radio conditions */
            if (!fstart) {
              fstart = 1;
              srslte_cell_fprint(stdout, &cell, sfn);
              cout << "*************************\n" <<
                      "*************************\n" <<
                      "Recording started: SFN: " << sfn << ", offset " << sfn_offset << "\n" <<
                      "*************************\n" <<
                      "*************************\n" << endl;
              if(netinfoBefore->nof_prb != cell.nof_prb && isZero(args.rf_freq)) {
                *netsync << "Mismatching nof_prb in aux modem " << netinfoBefore->nof_prb <<
                            " and sniffer " << cell.nof_prb << endl;
                go_exit = true;
              }
            }
            sfn = (sfn + static_cast<uint32_t>(sfn_offset)) % 1024;
          }
          else {
            *netsync << "Skipped illegal nof_prb=150" << endl;
          }
        }
        else {
          cout << "MIB not decoded. SFN: " << sfn <<
                  ", offset: " << sfn_offset << endl;
          if(!fstart) {
            if(sf_guard++ > DEFAULT_MIB_SEARCH_TIMEOUT_MS) {
              cout << "No MIB found in " << DEFAULT_MIB_SEARCH_TIMEOUT_MS << " subframes. Canceling..." << endl;
              go_exit = true;
            }
          }
        }
      }

      if (srslte_ue_sync_get_sfidx(&ue_sync) == 9) {
        sfn++;
      }

      if (fstart) {
        size_t nof_samples = SRSLTE_SF_LEN_PRB(cell.nof_prb);
        if(sink->write(sfb.sf_buffer[0], nof_samples) != nof_samples) {
          *netsync << "Writing to file sink failed (out of memory?)" << endl;
          go_exit = true;
        }
      }
    }
    else if (ret == 0) {
      if (fstart) {
        *netsync << "Sync loss at " << sfn << endl;
        go_exit = true;
        fstart = false;
      }
      else {
        cout << "Finding PSS... Peak: " << srslte_sync_get_peak_value(&ue_sync.sfind) <<
                ", FrameCnt: " << ue_sync.frame_total_cnt <<
                " State: " << ue_sync.state << endl;
//        fprintf(stderr,"Finding PSS... Peak: %8.1f, FrameCnt: %d, State: %d\n",
//                static_cast<double>(srslte_sync_get_peak_value(&ue_sync.sfind)),
//                ue_sync.frame_total_cnt, ue_sync.state);
      }
    }

    if (fstart) sf_cnt++;
//    sf_guard++;
//    if (sf_guard > nof_subframes + 10000) {
//      fprintf(stderr,"watchdog exit\n");
//      go_exit = true;
//    }
    if (sf_cnt == args.probing_delay && fstart && !trafficGen->isBusy()) {
      netinfoProbingStart = new NetworkInfo(modem->getNetworkInfo());
      switch (args.direction) {
        case 0:
          *netsync << "Start probing upload of " << args.payload_size <<
                      "Bytes to/from " << args.url << endl;
          trafficGen->performUpload(args.payload_size, args.url);
          break;
        case 1:
          *netsync << "Start probing download of " << args.payload_size <<
                      "Bytes to/from " << args.url << endl;
          trafficGen->performDownload(args.payload_size, args.url);
          break;
        default:
          *netsync << "Transfer direction undefined: " << args.direction << endl;
          break;
      }
      if(args.tx_power_sample_interval > 0) {
        modem->startTXPowerSampling();
      }
    }
  } // Main loop

  *netsync << "Capturing ended" << endl;

  //wait for traffigGen to be ready or cancel
  *netsync << "Waiting for traffic generator" << endl;
  int timeout_ms = DEFAULT_PROBING_TIMEOUT_MS;
  while(trafficGen->isBusy()) {
    if(go_exit || (timeout_ms <= 0)) { /* raw capturing failed, abort */
      *netsync << "Canceling traffic generator" << (go_exit ? " (capture failed)" : " (timeout)") << endl;
      trafficGen->cancel();
    }
    usleep(100000);
    timeout_ms -= 100;
  }
  trafficGen->cleanup();

  //validate cell
  NetworkInfo netinfoAfter(modem->getNetworkInfo());
  if(netinfoProbingStart != nullptr &&
     netinfoProbingStart->isValid() &&
     netinfoAfter.isValid()) {
    if(netinfoAfter == *netinfoBefore &&
       *netinfoProbingStart == *netinfoBefore) {
      *netsync << "[OK] No cell change detected, assuming valid capture" << endl;
      ofstream netInfoFile;
      netInfoFile.open(cellInfoFileName);
      if(netInfoFile.is_open()) {
        netInfoFile << netinfoProbingStart->toCSV(',');
        netInfoFile << ',';
        netInfoFile << GPS::toCSV(gpsFix, ",");
        netInfoFile << ',';
        netInfoFile << args.probing_delay;

        netInfoFile.close();
      }
      else {
        *netsync << "[FAIL] Could not save cell info to file: " << cellInfoFileName << endl;
      }
    }
    else {
      *netsync << "[FAIL] Cell change detected, invalid capture" <<
                  "\nBefore:    " << netinfoBefore->toCSV(',') <<
                  "\nProbStart: " << netinfoProbingStart->toCSV(',') <<
                  "\nAfter:     " << netinfoAfter.toCSV(',') << endl;
    }
  }
  else {
   *netsync << "[FAIL] Could not get network info from aux modem" << endl;
  }

  // store tx power samples if requested
  if(args.tx_power_sample_interval > 0) {
    vector<int> txPowerSamples = modem->getTXPowerSamples();
    ofstream txPowerSamplesFile;
    txPowerSamplesFile.open(txPowerSamplesFileName);
    if(txPowerSamplesFile.is_open()) {
      for(vector<int>::iterator it = txPowerSamples.begin(); it != txPowerSamples.end(); ++it) {
        if(it != txPowerSamples.begin()) {
          txPowerSamplesFile << endl;
        }
        txPowerSamplesFile << *it;
      }
      txPowerSamplesFile.close();
    }
  }

  *netsync << "Closing sink" << endl;
  sink->close();

  *netsync << "Closing receiver" << endl;
  srslte_ue_dl_free(&ue_dl);
  srslte_ue_sync_free(&ue_sync);
  srslte_ue_mib_free(&ue_mib);
  srslte_rf_kill_gain_thread(&rf);
  srslte_rf_close(&rf);

  delete netinfoBefore;
  delete netinfoProbingStart;
  cout << "\nBye" << endl;
  return true;
}

void CaptureProbeCore::handleSignal() {
  *netsync << "CaptureProbeCore: Exiting..." << endl;
  go_exit = true;
}
