#include "falcon/meas/NetsyncMaster.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

int main(int argc, char** argv) {
  NetsyncMaster* netsync = new NetsyncMaster("127.255.255.255", 4567);
  netsync->launchReceiver();
  netsync->start("DummID", 0);
  netsync->stop();
  netsync->poll();
  sleep(30);
  printf("Terminating recv thread...\n");
  delete netsync;
  printf("Deleted netsync\n");

  return EXIT_SUCCESS;
}
