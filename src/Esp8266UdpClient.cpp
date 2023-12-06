#include "Esp8266UdpClient.h"

// PUBLIC:
Esp8266UdpClient::Esp8266UdpClient() : mUdp(WiFiUDP()) {}

uint8_t Esp8266UdpClient::begin(uint16_t port) { return mUdp.begin(port); }

int Esp8266UdpClient::parsePacket() { return mUdp.parsePacket(); }

int Esp8266UdpClient::read(unsigned char *buffer, size_t len) {
  return mUdp.read(buffer, len);
}

int Esp8266UdpClient::beginPacket(const char *host, uint16_t port) {
  return mUdp.beginPacket(host, port);
}

size_t Esp8266UdpClient::write(const uint8_t *buffer, size_t size) {
  return mUdp.write(buffer, size);
}

int Esp8266UdpClient::endPacket() { return mUdp.endPacket(); }
