#pragma once

#include <Arduino.h>

class AbstractUdpClient {
 public:
  virtual uint8_t begin(uint16_t port) = 0;

  virtual int parsePacket() = 0;

  virtual int read(unsigned char *buffer, size_t len) = 0;
  virtual int beginPacket(const char *host, uint16_t port) = 0;
  virtual size_t write(const uint8_t *buffer, size_t size) = 0;
  virtual int endPacket() = 0;
};
