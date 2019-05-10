#pragma once

#include <time.h>
#include <sys/time.h>
#include <string>

class Stopwatch {
public:
  Stopwatch();
  Stopwatch(const Stopwatch&) = delete;
  Stopwatch& operator=(const Stopwatch&) = delete;
  ~Stopwatch() {}
  void start();
  timeval getAndRestart();
  timeval getAndContinue() const;
  static std::string toString(timeval t);
  static timeval subtract(const timeval& subtrahend, const timeval& minuend);
private:
  timeval timeStart;
  static void zero(timeval& t);
};

timeval operator-(const timeval& left, const timeval& right);
bool operator==(const timeval& left, const timeval& right);
bool operator<(const timeval& left, const timeval& right);
