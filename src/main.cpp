#include <Arduino.h>

#include "Debug.h"

void setup()
{
  Serial.begin(SERIAL_SPEED);
  while (!Serial)
  {
    delay(100);
  }

  D_Println(F("----Starting ESP----"));
}

void loop()
{
}