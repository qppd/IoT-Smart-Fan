#ifndef CURRENTSENSOR_H
#define CURRENTSENSOR_H

#include <Arduino.h>

class CURRENTSensor {
public:
    // Constructor with optimized defaults for ACS712 with voltage divider
    // pin: ESP32 ADC pin (typically pin 34)
    // vRef: ESP32 reference voltage (3.3V)
    // sensitivity: ACS712 sensitivity in mV/A (185 for 5A, 100 for 20A, 66 for 30A)
    // calibrationOffset: Current reading offset compensation
    // voltageDividerFactor: For 1kΩ+2kΩ divider = (1k+2k)/2k = 1.5
    CURRENTSensor(uint8_t pin, float vRef = 3.3, float sensitivity = 185.0, float calibrationOffset = 0.3, float voltageDividerFactor = 1.5);
    void begin();
    float readCurrent(uint16_t samples = 100);                                    // Legacy method for compatibility
    float readCurrentRMS(uint32_t sampleTimeMs = 1000);                         // Improved RMS measurement
    float getVPP(uint32_t sampleTimeMs = 1000);                                 // Peak-to-peak voltage measurement
    float calculatePower(float currentRMS, float voltage = 240.0, float calibrationFactor = 1.2); // Power calculation
    
private:
    uint8_t _pin;
    float _vRef;
    float _sensitivity;              // mV/A for ACS712 (185 for 5A, 100 for 20A, 66 for 30A)
    float _calibrationOffset;       // Current offset calibration
    float _voltageDividerFactor;    // Voltage divider ratio (for 1k and 2k resistors: 3k/2k = 1.5)
};

#endif // CURRENTSENSOR_H
