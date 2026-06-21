#ifndef _SEEED_XIAO_S3_WIO_H_
#define _SEEED_XIAO_S3_WIO_H_

// Verwijder de D0 definitie volledig; het platform regelt dit al.

// Batterij definities
#define PIN_VBAT                (1)     // Fysieke pin D0 (GPIO 1)
#define ADC_MULTIPLIER          (2.0F)  // Jouw deler (1M/1M)
#define ADC_RESOLUTION          (12)    // 12-bit ADC
#define AREF_VOLTAGE            (3.3)   // De referentie spanning van de ESP32-S3

// Power management boot protection
#define PWRMGT_VOLTAGE_BOOTLOCK    3300   // Won't boot below this voltage

#endif
