#pragma once

#include "Stopwatch.h"

class Lifetime;

class LifetimeCollector {
public:
  virtual void collect(Lifetime& lt) = 0;
  virtual ~LifetimeCollector();
};

class Lifetime {
public:
  Lifetime(LifetimeCollector& collector, const std::string& prefixText = "");
  Lifetime(const Lifetime&) = delete;
  Lifetime& operator=(const Lifetime&) = delete;
  virtual ~Lifetime();
  timeval getLifetime();
  std::string getLifetimeString();
  const std::string& getPrefixText() const;
  void setPrefixText(const std::string& prefixText);
private:
  LifetimeCollector& collector;
  Stopwatch stopwatch;
  std::string prefixText;
};

class PrintLifetime : public Lifetime {
public:
  PrintLifetime(const std::string& prefixText = "");
  PrintLifetime(const PrintLifetime&) = delete;
  PrintLifetime& operator=(const PrintLifetime&) = delete;
  ~PrintLifetime() override;
};

class GlobalLifetimePrinter : public LifetimeCollector {
public:
  static GlobalLifetimePrinter& getInstance();
  void collect(Lifetime& lt) override;
private:
  GlobalLifetimePrinter();
  static GlobalLifetimePrinter* instance;
};
