#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

#include "AbstractUdpClient.h"

class Esp8266UdpClient : public AbstractUdpClient {
 public:
  Esp8266UdpClient();

  uint8_t begin(uint16_t port);

  int parsePacket();

  int read(unsigned char *buffer, size_t len);
  int beginPacket(const char *host, uint16_t port);
  size_t write(const uint8_t *buffer, size_t size);
  int endPacket();

 private:
  WiFiUDP mUdp;
};
