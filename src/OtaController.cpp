#include "OtaController.h"

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

AsyncWebServer server{80};

// PUBLIC:
void OtaController::setup() {
  LittleFS.begin();
  ArduinoOTA.begin(false);
  ArduinoOTA.onStart(std::bind(&OtaController::handleOtaStart, this));
  ArduinoOTA.onEnd(std::bind(&OtaController::handleOtaEnd, this));

  server.begin();
  server.serveStatic("/", LittleFS, "website/index.html");
  server.on("/status", [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", "ESP Online");
  });

  mCurrentState = Setup;
}

void OtaController::loop() {
  switch (mCurrentState) {
    case Uninitialized:
      return;
    case Setup:
      ArduinoOTA.handle();
      break;
  }
}

// PRIVATE:
void OtaController::handleOtaStart() {
  // TODO: Handle ota start
}

void OtaController::handleOtaProgress(size_t current, size_t final) {
  // TODO: Handle ota progress
}

void OtaController::handleOtaEnd() {
  // TODO: Handle ota end
}
