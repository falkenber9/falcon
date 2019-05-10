#pragma once

#include "GPS.h"
#include "AuxModem.h"
#include "probe_modem.h"

class AuxModemGPS : public GPS {
public:
  AuxModemGPS();
  AuxModemGPS(const AuxModemGPS&) = delete; //prevent copy
  AuxModemGPS& operator=(const AuxModemGPS&) = delete; //prevent copy
  virtual ~AuxModemGPS();
  bool init(AuxModem* modem);
  GPSFix getFix();
private:
  AuxModem* modem;
};
