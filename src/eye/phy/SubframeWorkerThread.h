#pragma once

#include "SubframeWorker.h"
#include "falcon/common/ThreadSafeQueue.h"

#include "srslte/common/threads.h"

class SubframeWorkerThread : public thread {
public:
    SubframeWorkerThread(ThreadSafeQueue<SubframeWorker>& avail,
                         ThreadSafeQueue<SubframeWorker>& pending);
    virtual ~SubframeWorkerThread();
    void cancel();
    void wait_thread_finish();
protected:
  virtual void run_thread() override;
private:
  ThreadSafeQueue<SubframeWorker>& avail;
  ThreadSafeQueue<SubframeWorker>& pending;
  volatile bool canceled;
  volatile bool joined;
};
