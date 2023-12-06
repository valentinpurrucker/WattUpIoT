#pragma once

#include <Arduino.h>

#include "AbstractSerialDevice.h"

class HardwareSerialDevice : public AbstractSerialDevice {
 public:
  HardwareSerialDevice(HardwareSerial &serial);

  int available();

  int read();

 private:
  HardwareSerial &mSerial;
};
