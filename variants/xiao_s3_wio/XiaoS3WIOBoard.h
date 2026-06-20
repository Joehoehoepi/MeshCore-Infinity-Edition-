#pragma once

#include <Arduino.h>
#include <helpers/ESP32Board.h>

class XiaoS3WIOBoard : public ESP32Board {
public:
  XiaoS3WIOBoard() { }

  void begin(); // Initialisatie
    uint16_t getBattMilliVolts(); // De functie waar de app op wacht
  const char* getManufacturerName() const override {
    return "Xiao S3 WIO - Infinity";
  }
};
