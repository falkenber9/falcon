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
#include "falcon/meas/AuxModemGPS.h"

AuxModemGPS::AuxModemGPS() {
  modem = nullptr;
}

AuxModemGPS::~AuxModemGPS() {

}

bool AuxModemGPS::init(SierraWirelessAuxModem* modem) {
  this->modem = modem;
  if(modem == nullptr) return false;
  if(modem->getModemHandle() == nullptr) return false;

  if(start_gps(modem->getModemHandle()) != 0) return false;

  return true;
}

GPSFix AuxModemGPS::getFix() {
  GPSFix result;
  result.is_invalid = true;
  modem_t* hModem = modem->getModemHandle();
  sw_em7565_gpsloc_response_t location;
  if(0 == get_gps_fix(hModem, &location)) {
    result.latitude = location.latitude;
    result.longitude = location.longitude;
    result.altitude = location.altitude;
    result.heading = location.heading;
    result.velocity_h = location.velocity_h;
    result.velocity_v = location.velocity_v;
    result.is_invalid = location.is_invalid;
  }

  return result;
}
