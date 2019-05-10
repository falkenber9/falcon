#include "falcon/common/SignalManager.h"

#include <algorithm>
#include <signal.h>

SignalGate* SignalGate::instance = nullptr;

SignalHandler::SignalHandler() :
  gate(nullptr)
{

}

SignalHandler::~SignalHandler() {
  if(gate != nullptr) {
    gate->detach(*this);
    gate = nullptr;
  }
}

void SignalHandler::registerGate(SignalGate& gate) {
  this->gate = &gate;
}

void SignalHandler::deregisterGate() {
  this->gate = nullptr;
}



SignalGate::SignalGate() :
  handlers()
{

}

void SignalGate::notify() {
  for(std::vector<SignalHandler*>::iterator it = handlers.begin(); it != handlers.end(); ++it) {
    (*it)->handleSignal();
  }
}

SignalGate& SignalGate::getInstance() {
  if(instance == nullptr) {
    instance = new SignalGate();
  }
  return *instance;
}

void SignalGate::signalEntry(int sigNo) {
  if (sigNo == SIGINT) {
    getInstance().notify();
  }
}

SignalGate::~SignalGate() {
  for(std::vector<SignalHandler*>::iterator it = handlers.begin(); it != handlers.end(); ++it) {
    (*it)->deregisterGate();
  }
}

void SignalGate::init() {
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigprocmask(SIG_UNBLOCK, &sigset, nullptr);
  signal(SIGINT, signalEntry);
}

void SignalGate::attach(SignalHandler& handler) {
  handler.registerGate(*this);
  handlers.push_back(&handler);
}

void SignalGate::detach(SignalHandler& handler) {
  handler.deregisterGate();
  handlers.erase(std::remove(handlers.begin(), handlers.end(), &handler), handlers.end());
}

