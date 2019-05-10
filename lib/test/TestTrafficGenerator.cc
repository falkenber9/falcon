#include "falcon/meas/TrafficGenerator.h"
#include "falcon/meas/DummyEventHandler.h"
#include "falcon/meas/TrafficResultsToFile.h"

#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>

// this code makes use of assert(EXPRESSION).
// Since it drops EXPRESSION if NDEBUG is defined,
// i.e. in release mode, we undefine it here...
#undef NDEBUG
#include <assert.h>

using namespace std;

bool isZero(double value) {
  const double epsilon = 1e-5;
  return (value < epsilon) && (value > -epsilon);
}

void testUpload() {
  cout << "Testing regular upload" << endl;
  TrafficGenerator tg;
  assert(tg.isBusy() == false);
  ProbeResult res(tg.getStatus());
  assert(res.state == transfer_state_t::TS_UNDEFINED);
  assert(isZero(res.datarate_dl));
  assert(isZero(res.datarate_ul));
  assert(isZero(res.total_transfer_time));
  assert(res.payload_size == 0);

  assert(tg.performUpload(1e6, "http://129.217.211.19:6137/index.html"));

  while(tg.isBusy()) {
    usleep(100000);
  }
  // Final results
  res = tg.getStatus();
  assert(res.state == transfer_state_t::TS_FINISHED);
  cout << "Datarate DL/UL [B/s]: " << res.datarate_dl << "/" << res.datarate_ul << endl;
  cout << "Total transmission time [s]: " << res.total_transfer_time << endl;
  cout << "CSV: " << res.toCSV(',') << endl;

  cout << "Upload OK" << endl;
  tg.cleanup();
}

void testDownloadRegular() {
  cout << "Testing regular download" << endl;
  TrafficGenerator tg;
  assert(tg.isBusy() == false);
  ProbeResult res(tg.getStatus());
  assert(res.state == transfer_state_t::TS_UNDEFINED);
  assert(isZero(res.datarate_dl));
  assert(isZero(res.datarate_ul));

  assert(tg.performDownload(20e6, "http://129.217.211.19:6137/testfiles/1MB.bin"));

  while(tg.isBusy()) {
    usleep(100000);
  }
  // Final results
  res = tg.getStatus();
  assert(res.state == transfer_state_t::TS_FINISHED);
  cout << "Datarate DL/UL [B/s]: " << res.datarate_dl << "/" << res.datarate_ul << endl;
  cout << "Total transmission time [s]: " << res.total_transfer_time << endl;
  cout << "CSV: " << res.toCSV(',') << endl;

  cout << "Download OK" << endl;
  tg.cleanup();
}

void testDownloadTruncated() {  /* download 20MB file, truncate to 1MB */
  cout << "Testing truncated download" << endl;
  TrafficGenerator tg;
  assert(tg.isBusy() == false);
  ProbeResult res(tg.getStatus());
  assert(res.state == transfer_state_t::TS_UNDEFINED);
  assert(isZero(res.datarate_dl));
  assert(isZero(res.datarate_ul));

  assert(tg.performDownload(1e6, "http://129.217.211.19:6137/testfiles/20MB.bin"));

  while(tg.isBusy()) {
    usleep(100000);
  }
  // Final results
  res = tg.getStatus();
  assert(res.state == transfer_state_t::TS_FINISHED);
  cout << "Datarate DL/UL [B/s]: " << res.datarate_dl << "/" << res.datarate_ul << endl;
  cout << "Total transmission time [s]: " << res.total_transfer_time << endl;
  cout << "CSV: " << res.toCSV(',') << endl;

  cout << "Download OK" << endl;
  tg.cleanup();
}

void testDownloadWithEventHandler() {
  cout << "Testing regular download with event handler" << endl;
  TrafficGenerator tg;
  tg.setEventHandler(new DummyEventHandler());
  assert(tg.isBusy() == false);
  ProbeResult res(tg.getStatus());
  assert(res.state == transfer_state_t::TS_UNDEFINED);
  assert(isZero(res.datarate_dl));
  assert(isZero(res.datarate_ul));

  assert(tg.performDownload(20e6, "http://129.217.211.19:6137/testfiles/1MB.bin"));

  while(tg.isBusy()) {
    usleep(100000);
  }
  // Final results
  res = tg.getStatus();
  assert(res.state == transfer_state_t::TS_FINISHED);
  cout << "Datarate DL/UL [B/s]: " << res.datarate_dl << "/" << res.datarate_ul << endl;
  cout << "Total transmission time [s]: " << res.total_transfer_time << endl;
  cout << "CSV: " << res.toCSV(',') << endl;

  cout << "Download OK" << endl;
  tg.cleanup();
}

void testDownloadAndSaveResults() {
  cout << "Testing regular download with event handler" << endl;
  TrafficGenerator tg;
  tg.setEventHandler(new TrafficResultsToFile(&tg, "/tmp/deleteme.txt"));
  assert(tg.isBusy() == false);
  ProbeResult res(tg.getStatus());
  assert(res.state == transfer_state_t::TS_UNDEFINED);
  assert(isZero(res.datarate_dl));
  assert(isZero(res.datarate_ul));

  assert(tg.performDownload(20e6, "http://129.217.211.19:6137/testfiles/1MB.bin"));

  while(tg.isBusy()) {
    usleep(100000);
  }
  // Final results
  res = tg.getStatus();
  assert(res.state == transfer_state_t::TS_FINISHED);
  cout << "Datarate DL/UL [B/s]: " << res.datarate_dl << "/" << res.datarate_ul << endl;
  cout << "Total transmission time [s]: " << res.total_transfer_time << endl;
  cout << "CSV: " << res.toCSV(',') << endl;

  cout << "Download OK" << endl;
  tg.cleanup();
}

int main(int argc, char** argv) {
  (void)argc; //unused
  (void)argv; //unused

  testUpload();
  testDownloadRegular();
  testDownloadTruncated();
  testDownloadWithEventHandler();
  testDownloadAndSaveResults();
}
