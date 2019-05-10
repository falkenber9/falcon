#include "falcon/meas/AuxModem.h"

#include <iostream>
#include <memory>
#include <assert.h>

using namespace std;

int main(int argc, char** argv) {
  AuxModem modem;
  assert(modem.init());
  assert(modem.configure());
  {
  NetworkInfo infoA(modem.getNetworkInfo());
  NetworkInfo infoB(modem.getNetworkInfo());
  assert(infoA == infoB);
  }
  cout << "OK" << endl;
}
