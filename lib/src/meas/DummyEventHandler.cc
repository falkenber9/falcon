#include "falcon/meas/DummyEventHandler.h"

#include <iostream>

using namespace std;

void DummyEventHandler::actionBeforeTransfer() {
  cout << "DummyEventHandler: actionBeforeTransfer()" << endl;
}

void DummyEventHandler::actionDuringTransfer() {
  cout << "DummyEventHandler: actionDuringTransfer()" << endl;
}

void DummyEventHandler::actionAfterTransfer() {
  cout << "DummyEventHandler: actionAfterTransfer()" << endl;
}
