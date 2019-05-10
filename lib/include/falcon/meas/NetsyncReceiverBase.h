#pragma once

#include "NetsyncCommon.h"
#include <pthread.h>
#include <signal.h>

class NetsyncReceiverBase {
public:
  NetsyncReceiverBase();
  virtual ~NetsyncReceiverBase();
  void parse(const char* msg, size_t len);
  void launchReceiver();
protected:
  virtual void handle(NetsyncMessageStart msg);
  virtual void handle(NetsyncMessageStop msg);
  virtual void handle(NetsyncMessagePoll msg);
  virtual void handle(NetsyncMessageText msg);
private:
  virtual void receive() = 0;
  static void* receiveStart(void* obj);

  pthread_t recvThread;

protected:
  char* recvThreadBuf;
  size_t recvThreadBufSize;
  volatile bool cancelThread;
};
