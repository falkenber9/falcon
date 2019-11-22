#include "SubframeWorkerThread.h"
#include "falcon/prof/Lifetime.h"

#include <iostream>

SubframeWorkerThread::SubframeWorkerThread(ThreadSafeQueue<SubframeWorker>& avail,
                                           ThreadSafeQueue<SubframeWorker>& pending) :
  avail(avail),
  pending(pending),
  canceled(false),
  joined(false)
{

}

SubframeWorkerThread::~SubframeWorkerThread() {
  cancel();
  wait_thread_finish();
}

void SubframeWorkerThread::cancel() {
  // this function must not block!
  canceled = true;
}

void SubframeWorkerThread::wait_thread_finish() {
  if(!joined) {
    joined = true;
    thread::wait_thread_finish();
  }
}

void SubframeWorkerThread::run_thread() {
  std::cout << "SubframeWorkerThread ready" << std::endl;
  while(!canceled) {
    std::shared_ptr<SubframeWorker> worker = pending.dequeue();
    if(worker != nullptr) {
      worker->work();
      // enqueue finished worker
      avail.enqueue(std::move(worker));
    }
    else {
      // nullptr is only returned if phy is canceled
      canceled = true;
    }
  }
  std::cout << "SubframeWorkerThread ended" << std::endl;
}
