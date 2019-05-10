#include "falcon/meas/AuxModemGPS.h"

AuxModemGPS::AuxModemGPS() {
  modem = nullptr;
}

AuxModemGPS::~AuxModemGPS() {

}

bool AuxModemGPS::init(AuxModem* modem) {
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
