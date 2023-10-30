#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

class WifiManager {
  using OnConnectedCallback = std::function<void()>;

 public:
  enum WiFiState {
    Uninitialized,
    Connected,
    Disconnected,
    Connecting,
    Timeout,
    Failed
  };

  WifiManager();

  void setup();

  void loop();

  bool isConnected();

  bool addConnectionHandler(OnConnectedCallback onConnected);

  OnConnectedCallback mOnConnectedHandlers[10];

 private:
  void connectWiFi();

  void onDisconnect(const WiFiEventStationModeDisconnected &event);

  void onGotIP(const WiFiEventStationModeGotIP &event);

  void checkTimeout();

  void reconnect();

  WiFiClient mClient;

  WiFiState mCurrentState = WiFiState::Uninitialized;

  WiFiEventHandler mConnectedHandler;
  WiFiEventHandler mDisconnectedHandler;

  int mLastConnectionAttemptTs = 0;

  int mReconnectCounter = 0;
};
