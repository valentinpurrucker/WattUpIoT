#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

class AbstractWiFiClient {
 public:
  virtual void begin(const char *ssid, const char *password) = 0;
  virtual void mode(WiFiMode mode) = 0;

  virtual WiFiEventHandler onGotIP(
      std::function<void(const WiFiEventStationModeGotIP &event)>
          onGotIPCallback) = 0;
  virtual WiFiEventHandler onDisconnected(
      std::function<void(const WiFiEventStationModeDisconnected &event)>
          onDisconnectedCallback) = 0;
};
