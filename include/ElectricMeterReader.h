
#pragma once

#include <Arduino.h>
#include <memory.h>
#include <sml/sml_file.h>

#include "AbstractSerialDevice.h"
#include "MqttPublisher.h"
#include "Scheduler.h"

struct ObisCode {
  u8_t mediumCode;
  u8_t channel;
  u8_t physicalUnit;
  u8_t measurementType;
  u8_t tarrif;
  u8_t other;
};

struct EnergyData {
  double positiveEnergy = 0;
  double negativEnergy = 0;
  double currentPower = 0;
};

class ElectricMeterReader {
 public:
  static const int MAX_TRANSMISSION_DURATION_MS;
  static const int MAX_READ_TIME;

  static const byte SML_START_SEQ[8];
  static const byte SML_END_SEQ[5];

  enum EnergyMeterObisCodeType {
    MANUFACTURER,
    DEVICE_ID,
    ENERGY_POSITIVE,
    ENERGY_NEGATIV,
    CURRENT_ACTIVE_POWER
  };

  enum EnergyMeterState {
    Uninitialized,
    Idle,
    Waiting,
    Reading,
    DataRead,
  };

  enum ReadingState {
    NotReady,
    ReadingStart,
    ReadingMessage,
    ReadingChecksum,
    ReadingFinished,
    Timeout,
  };

  ElectricMeterReader(AbstractSerialDevice &serial, MqttPublisher &mqtt,
                      Scheduler &scheduler);

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

  void parseSmlData();

  void updateEnergyData(std::optional<EnergyData> &data,
                        EnergyMeterObisCodeType type, double value);

  static ObisCode getObisCodeFromType(EnergyMeterObisCodeType type);

  static std::optional<EnergyMeterObisCodeType>
  getEnergyMeterObisCodeTypeFromCode(ObisCode &&code);

  static const char *getTypeStringFromObisType(EnergyMeterObisCodeType type);

  EnergyMeterState mCurrentState = Uninitialized;

  ReadingState mReadingState = NotReady;

  AbstractSerialDevice &mSerial;

  int mLastReadingStartTs = -1;

  int mBaudRate = 9600;

  int mCurrentBufferPosition = 0;

  int mChecksumByteNumber = 3;

  int mBufferSize = 0;

  bool mDataReadCallbackCalled = false;

  bool mDataAvailableToSend = false;

  bool mDataParsed = false;

  std::unique_ptr<byte[]> mBuffer;

  std::optional<EnergyData> mCurrentData = std::nullopt;

  MqttPublisher &mMqtt;

  Scheduler &mScheduler;
};
