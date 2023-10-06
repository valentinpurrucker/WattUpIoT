
#pragma once

#include <Arduino.h>

struct ObisCode
{
    int8_t mediumCode;
    int8_t channel;
    int8_t physicalUnit;
    int8_t measurementType;
    int8_t tarrif;
    int8_t other;
};

class ElectricMeterReader
{

public:
    static const int MAX_TRANSMISSION_DURATION_MS;
    static const int MAX_READ_TIME;

    static const byte SML_START_SEQ[8];
    static const byte SML_END_SEQ[5];

    enum EnergyMeterObisCodeType
    {
        MANUFACTURER,
        DEVICE_ID,
        ENERGY_POSITIVE,
        ENERGY_NEGATIV,
        CURRENT_ACTIVE_POWER
    };

    enum EnergyMeterState
    {
        Uninitialized,
        Idle,
        Waiting,
        Reading,
        DataRead,
    };

    enum ReadingState
    {
        NotReady,
        ReadingStart,
        ReadingMessage,
        ReadingChecksum,
        ReadingFinished,
        Timeout,
    };

    void setup();

    bool update();

private:
    EnergyMeterState mCurrentState = Uninitialized;

    ReadingState mReadingState = NotReady;
};
