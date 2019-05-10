#include "falcon/prof/Lifetime.h"

#include <iostream>

using namespace std;

Lifetime::Lifetime(LifetimeCollector& collector, const string& prefixText) :
  collector(collector),
  prefixText(prefixText)
{
  stopwatch.start();
}

Lifetime::~Lifetime() {
  collector.collect(*this);
}

timeval Lifetime::getLifetime() {
  return stopwatch.getAndContinue();
}

string Lifetime::getLifetimeString() {
  return Stopwatch::toString(getLifetime());
}

const string& Lifetime::getPrefixText() const {
  return prefixText;
}

void Lifetime::setPrefixText(const string& prefixText) {
  this->prefixText = prefixText;
}

LifetimeCollector::~LifetimeCollector() {
  //nothing
}

GlobalLifetimePrinter* GlobalLifetimePrinter::instance = nullptr;
GlobalLifetimePrinter& GlobalLifetimePrinter::getInstance() {
  if(instance == nullptr) {
    instance = new GlobalLifetimePrinter();
  }
  return *instance;
}

void GlobalLifetimePrinter::collect(Lifetime& lt) {
  cout << lt.getPrefixText() << lt.getLifetimeString() << endl;
}

GlobalLifetimePrinter::GlobalLifetimePrinter() {
  //nothing
}

PrintLifetime::PrintLifetime(const string& prefixText) :
  Lifetime(GlobalLifetimePrinter::getInstance(), prefixText)
{
  //nothing
}

PrintLifetime::~PrintLifetime() {
  //work is done by virtual base-class destructor
}
