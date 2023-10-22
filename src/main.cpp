#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Esp.h>

#include "Debug.h"

#include "Scheduler.h"
#include "WiFiManager.h"
#include "MqttPublisher.h"
#include "NtpClient.h"
#include "ElectricMeterReader.h"
#include "OtaController.h"

Scheduler scheduler;

WifiManager wifiManager;

MqttPublisher mqttPublisher;

NtpClient ntpClient;

ElectricMeterReader eReader(&mqttPublisher, scheduler);

OtaController otaController;

Task wifiTask = {0, std::bind(&WifiManager::loop, &wifiManager), 1000ul, false, true};
Task mqttTask = {1, std::bind(&MqttPublisher::loop, &mqttPublisher), 1000ul, false, true};
Task ntpTask = {2, std::bind(&NtpClient::update, &ntpClient), 1000ul, false, true};
Task eReaderTask = {3, std::bind(&ElectricMeterReader::update, &eReader), 0ul, false, false};
Task eReaderReadTask = {4, std::bind(&ElectricMeterReader::sendDataIfAvailable, &eReader), 10000ul, false, true};
Task otaHandleTask = {5, std::bind(&OtaController::loop, &otaController), 5000ul, false, true};
Task eReaderWaitTask = {6, std::bind(&ElectricMeterReader::wait, &eReader), 5000ul, false, false};

void setup()
{
  Serial.begin(SERIAL_SPEED);
  while (!Serial)
  {
    delay(100);
  }

  D_Println(F("----Starting ESP----"));

  wifiManager.addConnectionHandler([&]()
                                   {
                                          ntpClient.setup();
                                                    ntpClient.requestTime(); });

  wifiManager.addConnectionHandler([&]()
                                   { mqttPublisher.setup(); });

  wifiManager.addConnectionHandler(std::bind([&]()
                                             { otaController.setup(); }));

  wifiManager.addConnectionHandler([&]() {});

  mqttPublisher.mOnConnectedHandler = []() {

  };

  ntpClient.mOnTimeReceivedCb = std::bind([](u_long time)
                                          { scheduler.setTimestamp(ntpClient.getUnixTimestamp());
                                          mqttPublisher.publish("data/system", "system online");
    eReader.wait(); },
                                          std::placeholders::_1);

  wifiManager.setup();
  eReader.setup();

  eReader.mOnDataReadCallback = [&]()
  {
    D_Println(F("Data read, calling callback"));

    eReader.wait();
  };

  scheduler.scheduleEvery(
      wifiTask.mId, []()
      { wifiManager.loop(); },
      wifiTask.mTime);

  scheduler.scheduleEvery(mqttTask.mId, mqttTask.mCallback, mqttTask.mTime);

  scheduler.scheduleEvery(ntpTask.mId, ntpTask.mCallback, ntpTask.mTime);

  scheduler.scheduleEvery(eReaderReadTask.mId, eReaderReadTask.mCallback, eReaderReadTask.mTime);
  scheduler.scheduleEvery(otaHandleTask.mId, otaHandleTask.mCallback, otaHandleTask.mTime);

  scheduler.scheduleRealtime(eReaderTask.mId, eReaderTask.mCallback);
}

void loop()
{
  scheduler.loop();
}