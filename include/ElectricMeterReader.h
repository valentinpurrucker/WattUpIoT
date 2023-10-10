
#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>

#include "MqttPublisher.h"
#include "Scheduler.h"

struct ObisCode
{
    int8_t mediumCode;
    int8_t channel;
    int8_t physicalUnit;
    int8_t measurementType;
    int8_t tarrif;
    int8_t other;
};

struct EnergyData
{
    double positiveEnergy = 0;
    double negativEnergy = 0;
    double currentPower = 0;
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

    ElectricMeterReader(Scheduler &scheduler);

    ElectricMeterReader(MqttPublisher *mqtt, Scheduler &scheduler);

    void setup();

    bool update();

    void reset();

    void read();

    void wait();

    void sendDataIfAvailable();

    void printMeterState();
    void printReadingState();

    std::function<void()> mOnDataReadCallback;

private:
    void resetReadingState();

    bool readSmlData();

    void readStartSequence();

    void readMessage();

    void readChecksum();

    bool checkTimeout();

    void updateEnergyData(std::optional<EnergyData> &data, EnergyMeterObisCodeType type, double value);

    ObisCode getObisCodeFromType(EnergyMeterObisCodeType type);

    std::optional<EnergyMeterObisCodeType> getEnergyMeterObisCodeTypeFromCode(ObisCode &&code);

    const char *getTypeStringFromObisType(EnergyMeterObisCodeType type);

    EnergyMeterState mCurrentState = Uninitialized;

    ReadingState mReadingState = NotReady;

    std::unique_ptr<SoftwareSerial> mEMeterSerial;

    int mLastReadingStartTs = -1;

    int mBaudRate = 9600;

    int mCurrentBufferPosition = 0;

    int mChecksumByteNumber = 3;

    int mBufferSize = 0;

    bool mDataReadCallbackCalled = false;

    bool mDataAvailableToSend = false;

    bool mDataParsed = false;

    std::unique_ptr<byte[]>
        mBuffer;

    std::optional<EnergyData> mCurrentData = std::nullopt;

    MqttPublisher *mMqtt;

    Scheduler &mScheduler;
};
