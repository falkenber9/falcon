#include "Phy.h"

#include <iostream>

Phy::Phy(uint32_t nof_rx_antennas, uint32_t nof_workers, const std::string& dciFilenName, const std::string& statsFileName, bool skipSecondaryMetaFormats, double metaFormatSplitRatio) :
  nof_rx_antennas(nof_rx_antennas),
  nof_workers(nof_workers),
  common(FALCON_MAX_PRB, nof_rx_antennas, dciFilenName, statsFileName),
  metaFormats(nof_falcon_ue_all_formats, metaFormatSplitRatio),
  workers(),
  workerThread(avail, pending)
{
  std::cout << "Creating Phy" << std::endl;
  metaFormats.setSkipSecondaryMetaFormats(skipSecondaryMetaFormats);

  for(uint32_t i=0; i<nof_workers; i++) {
    std::shared_ptr<SubframeWorker> worker(new SubframeWorker(i, common.max_prb, common, metaFormats));
    workers.push_back(worker);
    avail.enqueue(worker);
  }
  workerThread.start();
}

Phy::~Phy() {
  workerThread.cancel();
  avail.cancel();
  pending.cancel();
  workerThread.wait_thread_finish();

  // cleanup
  std::shared_ptr<SubframeWorker> buffer = nullptr;
  do {
    buffer = avail.dequeueImmediate();
  } while(buffer != nullptr);
  do {
    buffer = pending.dequeueImmediate();
  } while(buffer != nullptr);

  std::cout << "Destroyed Phy" << std::endl;
}

std::shared_ptr<SubframeWorker> Phy::getAvail() {
  return avail.dequeue();
}

std::shared_ptr<SubframeWorker> Phy::getAvailImmediate() {
  return avail.dequeueImmediate();
}

void Phy::putAvail(std::shared_ptr<SubframeWorker> buffer) {
  avail.enqueue(std::move(buffer));
}

std::shared_ptr<SubframeWorker> Phy::getPending() {
  return pending.dequeue();
}

void Phy::putPending(std::shared_ptr<SubframeWorker> buffer) {
  pending.enqueue(std::move(buffer));
}

// Wait until pending queue has been processed entirely and workerThread finished his work
void Phy::joinPending() {
  pending.waitEmpty();  // wait empty queue
  workerThread.cancel(); // mark worker thread as disabled
  pending.cancel(); // trigger cancel event to waiting worker thread
  workerThread.wait_thread_finish(); // wait worker thread to exit
}

PhyCommon& Phy::getCommon() {
  return common;
}

DCIMetaFormats& Phy::getMetaFormats() {
  return metaFormats;
}

std::vector<std::shared_ptr<SubframeWorker> >& Phy::getWorkers() {
  return workers;
}

bool Phy::setCell(srslte_cell_t cell) {
  bool result = true;
  for(auto& worker : workers) {
    if(!worker->setCell(cell)) {
      result = false;
    }
  }
  return result;
}

void Phy::setRNTI(uint16_t rnti) {
  for(auto& worker : workers) {
    worker->setRNTI(rnti);
  }
}

void Phy::setChestCFOEstimateEnable(bool enable, uint32_t mask) {
  for(auto& worker : workers) {
    worker->setChestCFOEstimateEnable(enable, mask);
  }
}

void Phy::setChestAverageSubframe(bool enable) {
  for(auto& worker : workers) {
    worker->setChestAverageSubframe(enable);
  }
}

void Phy::printStats() {
  common.printStats();
}
