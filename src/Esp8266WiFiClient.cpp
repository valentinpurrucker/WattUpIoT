// PUBLIC:

#include "Esp8266WiFiClient.h"

#include <ESP8266WiFi.h>

void Esp8266WiFiClient::begin(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);
}

void Esp8266WiFiClient::mode(WiFiMode mode) { WiFi.mode(mode); }

WiFiEventHandler Esp8266WiFiClient::onGotIP(
    std::function<void(const WiFiEventStationModeGotIP &event)>
        onGotIPCallback) {
  return WiFi.onStationModeGotIP(onGotIPCallback);
}

WiFiEventHandler Esp8266WiFiClient::onDisconnected(
    std::function<void(const WiFiEventStationModeDisconnected &event)>
        onDisconnectedCallback) {
  return WiFi.onStationModeDisconnected(onDisconnectedCallback);
}
