#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espMqttClient.h>

#include <functional>

#include "Scheduler.h"

class MqttPublisher {
  using OnConnectedCallback = std::function<void()>;

 public:
  enum MqttState {
    Uninitialized,
    WifiConnected,
    WifiDisconnected,
    MqttConnected,
    MqttConnecting,
    MqttDisconnected,
    MqttMessageSent,
    MqttMessageFailed,
    MqttConnectionTimeout
  };

  MqttPublisher() = default;

  void setup();
  void loop();

  void publish(char *topic, char *payload);

  OnConnectedCallback mOnConnectedHandler;

 private:
  void onWiFiConnected(const WiFiEventStationModeGotIP &event);

  void onWiFiDisconnected(const WiFiEventStationModeDisconnected &event);

  void onMqttConnected(bool sessionPresent);

  void onMqttDisconnected(espMqttClientTypes::DisconnectReason reason);

  void onMqttPublish(uint16_t packetId);

  void checkWiFi();

  void connectMqtt();

  void checkTimeout();

  void checkTimeoutTimer();

  espMqttClient mClient;

  MqttState mCurrentState = Uninitialized;

  WiFiEventHandler mConnectedHandler;
  WiFiEventHandler mDisconnectedHandler;

  int mLastConnectionAttemptTs = 0;
  int mTimeoutReconnectTimerTs = 0;
  int mLastReadTimestamp = -1;
};
