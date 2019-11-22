#include "falcon/phy/falcon_ue/BufferPool.h"

BufferPool::BufferPool(uint32_t rf_nof_rx_ant, uint32_t nof_buffers) {
  for(uint32_t i=0; i<nof_buffers; i++) {
    pending.enqueue(std::shared_ptr<SubframeBuffer>(new SubframeBuffer(rf_nof_rx_ant)));
  }
}

BufferPool::~BufferPool() {
  //please make sure, this object gets destroyed after all buffers have been returned to any queue

  // wake waiting consumers - give them nullptr
  avail.cancel();
  pending.cancel();

  // cleanup
  std::shared_ptr<SubframeBuffer> buffer = nullptr;
  do {
    buffer = avail.dequeueImmediate();
  } while(buffer != nullptr);
  do {
    buffer = pending.dequeueImmediate();
  } while(buffer != nullptr);
}

std::shared_ptr<SubframeBuffer> BufferPool::getAvail() {
  return avail.dequeue();
}

void BufferPool::putAvail(std::shared_ptr<SubframeBuffer> buffer) {
  avail.enqueue(std::move(buffer));
}

std::shared_ptr<SubframeBuffer> BufferPool::getPending() {
  return pending.dequeue();
}

void BufferPool::putPending(std::shared_ptr<SubframeBuffer> buffer) {
  pending.enqueue(std::move(buffer));
}
