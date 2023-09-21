#include "MqttPublisher.h"
#include "Debug.h"

// PUBLIC:

MqttPublisher::MqttPublisher(Scheduler &scheduler)
{
}

MqttPublisher::MqttPublisher()
{
}

void MqttPublisher::setup()
{

    mClient.setServer(STR(MQTT_ADDRESS), MQTT_PORT);
    mClient.setCredentials(STR(MQTT_USER), STR(MQTT_PASSWORD));
    mClient.setClientId(STR(MQTT_CLIENT));
    mClient.onConnect(std::bind(&MqttPublisher::onMqttConnected, this, std::placeholders::_1));
    mClient.onDisconnect(std::bind(&MqttPublisher::onMqttDisconnected, this, std::placeholders::_1));

    checkWiFi();
}

void MqttPublisher::loop()
{

    mClient.loop();

    switch (mCurrentState)
    {
    case Uninitialized:
        return;
    case MqttConnecting:
        D_Println(F("mqtt connecting..."));
        checkTimeout();
        break;
    case WifiConnected:
        connectMqtt();
        break;
    case WifiDisconnected:
        checkWiFi();
        break;

    case MqttConnected:
        // D_Println("mqtt con");
        break;
    case MqttDisconnected:
        D_Println(F("mqtt discon"));
        checkTimeoutTimer();
        break;

    case MqttConnectionTimeout:
        D_Println(F("Mqtt Timeout"));
        checkTimeoutTimer();
        break;

    default:
        break;
    }
}

// PRIVATE:

void MqttPublisher::onMqttConnected(bool sessionPresent)
{
    mCurrentState = MqttConnected;

    if (mOnConnectedHandler)
    {
        mOnConnectedHandler();
    }
}

void MqttPublisher::onMqttDisconnected(espMqttClientTypes::DisconnectReason reason)
{
    mCurrentState = MqttDisconnected;
    mTimeoutReconnectTimerTs = millis();
}

void MqttPublisher::checkWiFi()
{
    if (WiFi.isConnected())
    {
        mCurrentState = WifiConnected;
    }
    else
    {
        mCurrentState = WifiDisconnected;
    }
}

void MqttPublisher::connectMqtt()
{
    mCurrentState = MqttConnecting;
    mLastConnectionAttemptTs = millis();
    mTimeoutReconnectTimerTs = 0;
    mClient.connect();
}

void MqttPublisher::checkTimeout()
{
    if (millis() - mLastConnectionAttemptTs > 8000)
    {
        mCurrentState = MqttConnectionTimeout;
        mTimeoutReconnectTimerTs = millis();
    }
}

void MqttPublisher::checkTimeoutTimer()
{
    if (millis() - mTimeoutReconnectTimerTs > 2000)
    {
        if (mClient.connected())
        {
            mClient.disconnect();
        }
        connectMqtt();
    }
}