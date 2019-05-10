#include "falcon/meas/GPS.h"

#include <sstream>

using namespace std;

GPS::GPS() {

}

GPS::~GPS() {

}

string GPS::toCSV(const GPSFix& obj, const string& delim) {
  stringstream stream;
  stream << obj.latitude << delim;
  stream << obj.longitude;
  // maybe other members, too

  return stream.str();
}


DummyGPS::DummyGPS() {

}

DummyGPS::~DummyGPS() {

}

GPSFix DummyGPS::getFix() {
  GPSFix result;
  result.is_invalid = true;
  return result;
}
