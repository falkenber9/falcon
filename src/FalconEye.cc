#include "eye/ArgManager.h"
#include "eye/EyeCore.h"

#include "falcon/version.h"

#include "falcon/common/SignalManager.h"

#include <iostream>
#include <memory>
#include <cstdlib>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv) {
  cout << "FalconEye, version: " << falcon_get_version_git() << endl;
  cout << "Copyright (C) 2019 Robert Falkenberg" << endl;
  cout << endl;

  Args args;
  ArgManager::parseArgs(args, argc, argv);

  //attach signal handlers (for CTRL+C)
  SignalGate& signalGate(SignalGate::getInstance());
  signalGate.init();

  EyeCore eye(args);
  signalGate.attach(eye);

  bool success = eye.run();

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
