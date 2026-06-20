#pragma once

#include <Arduino.h>
#include <helpers/ESP32Board.h>

// Geef de klasse een eigen unieke naam
class LilyGoT3S3SX1262Board : public ESP32Board {
public:
  LilyGoT3S3SX1262Board() : ESP32Board() { }

  // Nu overschrijf je alleen wat nodig is
  const char* getManufacturerName() const override {
    return "Lilygo T3S3 - Infinity";
  }
};