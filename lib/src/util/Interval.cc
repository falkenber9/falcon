#include "Interval.h"

Interval::Interval(uint16_t start_end) :
  start(start_end),
  end(start_end)
{

}

Interval::Interval(uint16_t start, uint16_t end) :
  start(start),
  end(end)
{

}

bool Interval::matches(uint16_t value) {
  return (value >= start && value <= end);
}
