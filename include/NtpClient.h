#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

class NtpClient
{

public:
    static const size_t NTP_PACKET_SIZE = 48;
    static const u_long UNIX_TIMESTAMP_OFFSET = 2208988800UL;
    static const u_long TIMEOUT_SEC = 5000;

    enum NtpClientState
    {
        Uninitialized,
        UdpSetup,
        AwaitingTimeResponse,
        SendingTimeRequest,
        TimeAvailable,
        Timeout,
    };

    void setup();

    void update();

    void requestTime();

    u_long getUnixTimestamp();

    std::function<void(u_long)> mOnTimeReceivedCb;

private:
    void waitForTimeResponse();

    bool checkTimeout();

    void sendNtpRequest();

    NtpClientState mCurrentState = Uninitialized;

    WiFiUDP mUdp{};

    byte mBuffer[NTP_PACKET_SIZE]{};

    u_long mTimestamp = 0;

    u_long mRequestTimestamp;
};
