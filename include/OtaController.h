#pragma once

#include <Arduino.h>
#include <ArduinoOTA.h>

class OtaController {
 public:
  enum OtaControllerState { Uninitialized, Setup };

  void setup();

  void loop();

 private:
  void handleOtaStart();
  void handleOtaProgress(size_t current, size_t final);
  void handleOtaEnd();

  OtaControllerState mCurrentState{Uninitialized};
};
