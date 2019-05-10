#pragma once

class Cancelable {
public:
  Cancelable();
  Cancelable(const Cancelable&) = delete; //prevent copy
  Cancelable& operator=(const Cancelable&) = delete; //prevent copy
  virtual ~Cancelable();

  virtual void cancel() = 0;
};
