#include "SoftwareSerialDevice.h"

// PUBLIC:

SoftwareSerialDevice::SoftwareSerialDevice(SoftwareSerial &serial)
    : mSerial(serial) {}

int SoftwareSerialDevice::available() { return mSerial.available(); }

int SoftwareSerialDevice::read() { return mSerial.read(); }
