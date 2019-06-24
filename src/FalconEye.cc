/*
 * Copyright (c) 2019 Robert Falkenberg.
 *
 * This file is part of FALCON 
 * (see https://github.com/falkenber9/falcon).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */
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
