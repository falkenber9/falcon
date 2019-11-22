#pragma once

#include <stdint.h>
#include <stdio.h>
#include <memory>

#include "PhyCommon.h"
#include "MetaFormats.h"
#include "SubframeWorkerThread.h"
#include "SubframeWorker.h"
#include "falcon/common/ThreadSafeQueue.h"

#include "srslte/common/common.h"

#define DEFAULT_NOF_WORKERS 20
#define FALCON_MAX_PRB 110

//Phy main object
class Phy {
public:
  Phy(uint32_t nof_rx_antennas,
      uint32_t nof_workers,
      const std::string& dciFilenName,
      const std::string& statsFileName,
      bool skipSecondaryMetaFormats,
      double metaFormatSplitRatio);
  ~Phy();
  std::shared_ptr<SubframeWorker> getAvail();
  std::shared_ptr<SubframeWorker> getAvailImmediate();
  void putAvail(std::shared_ptr<SubframeWorker>);
  std::shared_ptr<SubframeWorker> getPending();
  void putPending(std::shared_ptr<SubframeWorker>);
  void joinPending();
  PhyCommon& getCommon();
  DCIMetaFormats& getMetaFormats();
  std::vector<std::shared_ptr<SubframeWorker> >& getWorkers();
  bool setCell(srslte_cell_t cell);
  void setRNTI(uint16_t rnti);
  void setChestCFOEstimateEnable(bool enable, uint32_t mask);
  void setChestAverageSubframe(bool enable);

  void printStats();

  uint32_t nof_rx_antennas;
  uint32_t nof_workers;
private:
  PhyCommon common;
  DCIMetaFormats metaFormats;

  std::vector<std::shared_ptr<SubframeWorker>> workers;
  ThreadSafeQueue<SubframeWorker> avail;
  ThreadSafeQueue<SubframeWorker> pending;
  SubframeWorkerThread workerThread;

};
