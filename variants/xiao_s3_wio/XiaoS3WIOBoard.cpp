#ifdef SEEED_XIAO_S3

#include "XiaoS3WIOBoard.h"
#include "variant.h"
#include <Arduino.h>

// Initialisatie van het bord
void XiaoS3WIOBoard::begin() {
    
    // Roep EERST de basisklasse aan zodat de I2C-bus (Wire) netjes start voor je scherm!
    ESP32Board::begin();

    // Configureer de pin voor jouw 1M/1M deler
    pinMode(PIN_VBAT, INPUT);
    
    // Stel resolutie in (12-bit standaard voor ESP32-S3)
    analogReadResolution(12);
}

// Deze functie wordt aangeroepen door je app/firmware
uint16_t XiaoS3WIOBoard::getBattMilliVolts() {
    // Lees de ruwe waarde (0 - 4095)
    int adcvalue = analogRead(PIN_VBAT);
    
    // Berekening met variabelen uit variant.h
    // Formule: (waarde / max_adc) * referentie_spanning * vermenigvuldiger * 1000 (voor mV)
    float voltage = (adcvalue / 4095.0f) * AREF_VOLTAGE * ADC_MULTIPLIER * 1000.0f;
    
    // Geef terug als geheel getal (mV)
    return (uint16_t)voltage;
}

#endif
