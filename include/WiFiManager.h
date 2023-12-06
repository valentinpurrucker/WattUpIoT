#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "AbstractWiFiClient.h"

class WifiManager {
  using OnConnectedCallback = std::function<void()>;

 public:
  static const int8_t MAX_NUMBER_CALLBACKS = 10;
  static const int WIFI_CONNECTING_TIMEOUT = 5000;
  static const int8_t MAX_NUMBER_RECONNECT_TRIES = 5;

  enum WiFiState {
    Uninitialized,
    Connected,
    Disconnected,
    Connecting,
    Timeout,
    Failed
  };

  WifiManager(AbstractWiFiClient &wifiClient);

  void setup();

  void loop();

  bool isConnected();

  bool addConnectionHandler(OnConnectedCallback onConnected);

  OnConnectedCallback mOnConnectedHandlers[MAX_NUMBER_CALLBACKS];

 private:
  void connectWiFi();

  void onDisconnect(const WiFiEventStationModeDisconnected &event);

  void onGotIP(const WiFiEventStationModeGotIP &event);

  void checkTimeout();

  void reconnect();

  AbstractWiFiClient &mClient;

  WiFiState mCurrentState = WiFiState::Uninitialized;

  WiFiEventHandler mConnectedHandler;
  WiFiEventHandler mDisconnectedHandler;

  int mLastConnectionAttemptTs = 0;

  int mReconnectCounter = 0;
};
