#pragma once

#include <Arduino.h>

#include "AbstractWiFiClient.h"

class Esp8266WiFiClient : public AbstractWiFiClient {
 public:
  void begin(const char *ssid, const char *password);
  void mode(WiFiMode mode);

  WiFiEventHandler onGotIP(
      std::function<void(const WiFiEventStationModeGotIP &event)>
          onGotIPCallback);

  WiFiEventHandler onDisconnected(
      std::function<void(const WiFiEventStationModeDisconnected &event)>
          onDisconnectedCallback);
};
