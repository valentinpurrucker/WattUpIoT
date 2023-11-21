#include "HardwareSerialDevice.h"

// PUBLIC:

HardwareSerialDevice::HardwareSerialDevice(HardwareSerial &serial)
    : mSerial(serial) {}

int HardwareSerialDevice::available() { return mSerial.available(); }

int HardwareSerialDevice::read() { return mSerial.read(); }
