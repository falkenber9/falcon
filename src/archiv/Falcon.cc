
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <iostream>
#include <stdint.h>

#include "Falcon.h"

Falcon& Falcon::instance() {
  static Falcon theFalconMain;
  return theFalconMain;
}

Falcon::Falcon() {

}

Falcon::~Falcon() {

}
