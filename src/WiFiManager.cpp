
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Esp.h>

#include "WiFiManager.h"
#include "Debug.h"

// PUBLIC:
WifiManager::WifiManager()
{
}

void WifiManager::loop()
{
    switch (mCurrentState)
    {

    case Uninitialized:
        return;

    case Connecting:
        D_Println(F("WiFi Connecting"));
        checkTimeout();
        break;
    case Connected:
        D_Println(F("WiFi Connected"));
        break;

    case Disconnected:
        D_Println(F("WiFi Disconnected"));
        reconnect();
        break;

    case Timeout:
        D_Println(F("WiFi Timeout"));
        reconnect();
        break;
    case Failed:
        D_Println(F("Failed"));
        break;
    default:
        return;
    }
}

bool WifiManager::isConnected()
{
    return mCurrentState == Connected;
}

bool WifiManager::addConnectionHandler(OnConnectedCallback onConnected)
{
    for (int i = 0; i < 10; i++)
    {
        if (mOnConnectedHandlers[i] == nullptr)
        {
            mOnConnectedHandlers[i] = onConnected;
            return true;
        }
    }
    return false;
}

// PRIVATE:

void WifiManager::setup()
{
    mDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&WifiManager::onDisconnect, this, std::placeholders::_1));
    mConnectedHandler = WiFi.onStationModeGotIP(std::bind(&WifiManager::onGotIP, this, std::placeholders::_1));
    connectWiFi();
}

void WifiManager::connectWiFi()
{
    mCurrentState = Connecting;
    mLastConnectionAttemptTs = millis();
    WiFi.mode(WiFiMode_t::WIFI_STA);
    WiFi.begin(STR(WIFI_NAME), STR(WIFI_PASSWORD));
}

void WifiManager::onDisconnect(const WiFiEventStationModeDisconnected &event)
{
    mCurrentState = Disconnected;
}

void WifiManager::onGotIP(const WiFiEventStationModeGotIP &event)
{
    mCurrentState = Connected;
    mReconnectCounter = 0;

    for (int i = 0; i < 10; i++)
    {
        if (mOnConnectedHandlers[i])
        {
            mOnConnectedHandlers[i]();
        }
    }
}

void WifiManager::checkTimeout()
{
    if (millis() - mLastConnectionAttemptTs >= 5000)
    {
        mCurrentState = Timeout;
    }
}

void WifiManager::reconnect()
{
    mReconnectCounter++;

    if (mReconnectCounter > 5)
    {
        mCurrentState = Failed;
        ESP.restart();
        return;
    }

    mCurrentState = Connecting;
    mLastConnectionAttemptTs = millis();
}