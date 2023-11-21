#pragma once

#include <Arduino.h>

class AbstractSerialDevice {
 public:
  virtual int available() = 0;
  virtual int read() = 0;
};
