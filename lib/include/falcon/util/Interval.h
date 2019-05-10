#ifndef INTERVAL_H
#define INTERVAL_H

#include <stdint.h>

class Interval {
public:
  Interval(uint16_t start_end);
  Interval(uint16_t start, uint16_t end);
  bool matches(uint16_t value);
private:
  uint16_t start;
  uint16_t end;
};

#endif // INTERVAL_H
