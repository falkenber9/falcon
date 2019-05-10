#pragma once

#include "falcon/meas/NetsyncCommon.h"
#include "falcon/meas/NetsyncReceiverBase.h"
#include "falcon/meas/BroadcastSlave.h"
#include "falcon/meas/AuxModem.h"
#include "falcon/meas/Cancelable.h"
#include "falcon/common/SignalManager.h"
#include <pthread.h>
#include <signal.h>
#include <condition_variable>

class NetsyncSlave : public NetsyncReceiverBase, public SignalHandler {
public:
  NetsyncSlave(const uint16_t port);
  ~NetsyncSlave();
  void pollReply();
  void startReply();
  void stopReply();
  void notifyFinished();
  void notifyError(const std::string text);

  void init(AuxModem* modem);
  bool waitStart();
  void signalStart();
  void signalAbort();
  bool stopReceived();
  void attachCancelable(Cancelable* obj);

  const NetsyncMessageStart& getRemoteParams() const;
protected:
  void handle(NetsyncMessagePoll msg);
  void handle(NetsyncMessageStart msg);
  void handle(NetsyncMessageStop msg);
private:
  void replyText(const std::string& text);
  void receive();
  void signal(bool abort);

  void handleSignal();
  BroadcastSlave broadcastSlave;
  AuxModem* modem;
  std::mutex startMutex;
  std::condition_variable startConditionVar;
  bool startFlag;
  bool stopFlag;
  Cancelable* stopObject;
  NetsyncMessageStart remoteParams;
  stringstream textstream;

  template<typename T>
  friend NetsyncSlave& operator<<(NetsyncSlave &s, const T &obj);
  friend NetsyncSlave& endl(NetsyncSlave& s);
  friend NetsyncSlave& operator<<(NetsyncSlave &s, NetsyncSlave& (*f)(NetsyncSlave&));
  //friend NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ostream&));
  //friend NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ios&));
  //friend NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ios_base&));
};

template<typename T>
NetsyncSlave& operator<<(NetsyncSlave &s, const T &obj) {
  std::cout << obj;
  s.textstream << obj;
  return s;
}

NetsyncSlave& endl(NetsyncSlave& s);
NetsyncSlave& operator<<(NetsyncSlave &s, NetsyncSlave& (*f)(NetsyncSlave&));
//NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ostream&));
//NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ios&));
//NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ios_base&));
