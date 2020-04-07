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
#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>

#include "falcon/common/Settings.h"
#include "falcon/common/Version.h"

#include "falcon/meas/NetsyncController.h"
#include "falcon/meas/NetsyncMaster.h"

#include "falcon/meas/AuxModem.h"
#include "falcon/meas/AuxModemGPS.h"
#include "falcon/meas/GPS.h"

using namespace std;

struct Args {
  string ip;
  uint16_t port;
  uint32_t pollIntervalSec;
  uint32_t autoIntervalSec;
  uint32_t nofSubframes;
  uint32_t offsetSubframes;
  size_t payloadSize;
  string urlUL;
  string urlDL;
  string gpsDevice;
  bool gpsEnabled;
  uint32_t tx_power_sample_interval;
};

void defaultArgs(Args& args) {
  //args.ip = "129.217.186.255";  // CNI broadcast
  //args.ip = "127.255.255.255";  // localhost "broadcast"
  args.ip = "169.254.255.255";  // link-local broadcast
  args.port = DEFAULT_NETSYNC_PORT;
  args.pollIntervalSec = DEFAULT_POLL_INTERVAL_SEC;
  args.autoIntervalSec = DEFAULT_AUTO_INTERVAL_SEC;
  args.nofSubframes = DEFAULT_NOF_SUBFRAMES_TO_CAPTURE;
  args.offsetSubframes = DEFAULT_PROBING_DELAY_MS;
  args.payloadSize = DEFAULT_PROBING_PAYLOAD_SIZE;
  args.urlUL = DEFAULT_PROBING_URL_UPLINK;
  args.urlDL = DEFAULT_PROBING_URL_DOWNLINK;
  args.gpsDevice = "";
  args.gpsEnabled = true;
  args.tx_power_sample_interval = DEFAULT_TX_POWER_SAMPLING_INTERVAL_US;
}

void usage(Args& args, const std::string& prog) {
  cout << "Usage: " << prog << " [agGhijnopsTwW] " << endl;
  cout << "\t-a Broadcast IP address [default: " << args.ip << "]" << endl;
  cout << "\t-g NMEA GPS device path [default: use GPS from aux modem]" << endl;
  cout << "\t-G Disable GPS entirely" << endl;
  cout << "\t-h Show this help message" << endl;
  cout << "\t-i Poll interval in s [default: " << args.pollIntervalSec << "]" << endl;
  cout << "\t-j Auto mode interval in s [default: " << args.autoIntervalSec << "]" << endl;
  cout << "\t-n Number of subframes in ms [default: " << args.nofSubframes << "], 0=unlimited" << endl;
  cout << "\t-o Probing delay/offset in ms [default: " << args.offsetSubframes << "]" << endl;
  cout << "\t-p Broadcast port [default: " << args.port << "]" << endl;
  cout << "\t-s Payload size in byte [default: " << args.payloadSize << "]" << endl;
  cout << "\t-w Probing URL for upload [default: " << args.urlUL << "]" << endl;
  cout << "\t-W Probing URL for download [default: " << args.urlDL << "]" << endl;
  cout << "\t-T TX power sampling interval in us [default: " << args.tx_power_sample_interval << "], 0=disabled" << endl;
}

void parseArgs(int argc, char** argv, Args& args) {
  int opt;
  while ((opt = getopt(argc, argv, "agGhijnopsTwW")) != -1) {
    switch (opt) {
      case 'a':
        args.ip = argv[optind];
        break;
      case 'g':
        args.gpsDevice = argv[optind];
        break;
      case 'G':
        args.gpsEnabled = false;
        break;
      case 'i':
        args.pollIntervalSec = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'j':
        args.autoIntervalSec = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'n':
        args.nofSubframes = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'o':
        args.offsetSubframes = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'p':
        args.port = static_cast<uint16_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 's':
        args.payloadSize = strtoul(argv[optind], nullptr, 0);
        break;
      case 'T':
        args.tx_power_sample_interval = static_cast<uint32_t>(strtoul(argv[optind], nullptr, 0));
        break;
      case 'w':
        args.urlUL = argv[optind];
        break;
      case 'W':
        args.urlDL = argv[optind];
        break;
      case 'h':
      default:
        usage(args, argv[0]);
        exit(-1);
    }
  }
}

int main(int argc, char** argv) {
  cout << "FalconCaptureWarden, version " << Version::gitVersion() << endl;
  cout << "Copyright (C) 2020 Robert Falkenberg" << endl;
  cout << endl;

  Args args;
  defaultArgs(args);
  parseArgs(argc, argv, args);


  SierraWirelessAuxModem modem;
  AuxModemGPS gps;
  GPS* gpsRef = nullptr;
  if(args.gpsEnabled) {
    if(!modem.init()) {
      cout << "Warning: Could not initialize AuxModem - continue without GPS" << endl;
    }
    else if(!gps.init(&modem)) {
      cout << "Warning: Could not initialize GPS from AuxModem - continue without GPS" << endl;
    }
    else {
      gpsRef = &gps;
    }
  }
  else {
    cout << "Warning: GPS disabled by command line argument - continue without GPS" << endl;
  }

  NetsyncController netsyncController;
  shared_ptr<NetsyncMaster> netsyncMaster(new NetsyncMaster(args.ip, args.port));
  netsyncMaster->init(args.nofSubframes, args.offsetSubframes, args.payloadSize, args.urlUL, args.urlDL, gpsRef, args.tx_power_sample_interval);
  netsyncMaster->location();

  netsyncController.init(netsyncMaster, args.pollIntervalSec, args.autoIntervalSec);

  cout << endl;
  cout << "Enter commands or type 'help' to display a help message." << endl;
  cout << endl;
  do {
    cout << "> " << flush;
  } while(netsyncController.parse(cin));
  cout << "Bye" << endl;
  return 0;
}
