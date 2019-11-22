#pragma once

#include "falcon/common/ThreadSafeQueue.h"
#include "falcon/common/SubframeBuffer.h"

class BufferPool {

private:
    ThreadSafeQueue<SubframeBuffer> avail;
    ThreadSafeQueue<SubframeBuffer> pending;

public:
    BufferPool(uint32_t rf_nof_rx_ant, uint32_t nof_buffers);
    ~BufferPool();
    std::shared_ptr<SubframeBuffer> getAvail();
    void putAvail(std::shared_ptr<SubframeBuffer>);
    std::shared_ptr<SubframeBuffer> getPending();
    void putPending(std::shared_ptr<SubframeBuffer>);
};
