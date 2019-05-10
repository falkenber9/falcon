#include "falcon/meas/NetsyncSlave.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

int main(int argc, char** argv) {
  NetsyncSlave* netsync = new NetsyncSlave(4567);
  netsync->launchReceiver();

  sleep(120);
  printf("Terminating recv thread...\n");
  delete netsync;
  printf("Deleted netsync\n");

  return EXIT_SUCCESS;
}

