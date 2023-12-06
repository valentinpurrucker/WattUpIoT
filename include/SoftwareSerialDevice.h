#pragma once

#include <SoftwareSerial.h>

#include "AbstractSerialDevice.h"

class SoftwareSerialDevice : public AbstractSerialDevice {
 public:
  SoftwareSerialDevice(SoftwareSerial &serial);

  int available();

  int read();

 private:
  SoftwareSerial &mSerial;
};
