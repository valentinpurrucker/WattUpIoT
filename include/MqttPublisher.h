#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espMqttClient.h>
#include <functional>

#include "Scheduler.h"

class MqttPublisher
{
    using OnConnectedCallback = std::function<void()>;

public:
    enum MqttState
    {
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

    MqttPublisher();
    MqttPublisher(Scheduler &scheduler);

    void setup();
    void loop();

    OnConnectedCallback mOnConnectedHandler;

private:
    void onMqttConnected(bool sessionPresent);

    void onMqttDisconnected(espMqttClientTypes::DisconnectReason reason);

    void checkWiFi();

    void connectMqtt();

    void checkTimeout();

    void checkTimeoutTimer();

    espMqttClient mClient;

    MqttState mCurrentState = Uninitialized;

    int mLastConnectionAttemptTs = 0;
    int mTimeoutReconnectTimerTs = 0;
    int mLastReadTimestamp = -1;
};
