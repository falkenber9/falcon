#pragma once

#include <vector>

class SignalGate;

class SignalHandler {
public:
  SignalHandler();
  SignalHandler(const SignalHandler&) = delete; //prevent copy
  SignalHandler& operator=(const SignalHandler&) = delete; //prevent copy
  virtual ~SignalHandler();
private:
  friend class SignalGate;
  virtual void handleSignal() = 0;
  void registerGate(SignalGate& gate);
  void deregisterGate();
  SignalGate* gate;
};

class SignalGate {
public:
  static SignalGate& getInstance();
  static void signalEntry(int sigNo);
  virtual ~SignalGate();
  void init();
  void attach(SignalHandler& handler);
  void detach(SignalHandler& handler);
private:
  SignalGate();
  SignalGate(const SignalGate&) = delete; //prevent copy
  SignalGate& operator=(const SignalGate&) = delete; //prevent copy
  void notify();
  static SignalGate* instance;
  std::vector<SignalHandler*> handlers;
};
