#include "CURRENTSensor.h"

CURRENTSensor::CURRENTSensor(uint8_t pin, float vRef, float sensitivity, float calibrationOffset, float voltageDividerFactor)
    : _pin(pin), _vRef(vRef), _sensitivity(sensitivity), _calibrationOffset(calibrationOffset), _voltageDividerFactor(voltageDividerFactor) {}

void CURRENTSensor::begin() {
    pinMode(_pin, INPUT);
}

// Original method for backward compatibility
float CURRENTSensor::readCurrent(uint16_t samples) {
    float avg = 0;
    for (uint16_t i = 0; i < samples; i++) {
        avg += analogRead(_pin);
        delayMicroseconds(100);
    }
    avg /= samples;
    float voltage = (avg / 4095.0) * _vRef; // ESP32 ADC is 12-bit (0-4095)
    float offset = _vRef / 2.0; // ACS712 output is Vcc/2 at 0A
    float current = (voltage - offset) / (_sensitivity / 1000.0); // Convert mV/A to V/A
    return current;
}

// New RMS current measurement method with improved accuracy
float CURRENTSensor::readCurrentRMS(uint32_t sampleTimeMs) {
    float peakToPeakVoltage = getVPP(sampleTimeMs);
    
    // Apply voltage divider correction
    peakToPeakVoltage *= _voltageDividerFactor;
    
    // Convert peak-to-peak to RMS voltage
    float vrms = (peakToPeakVoltage / 2.0) * 0.707; // RMS = Peak/√2, and Peak = P-P/2
    
    // Convert to current using ACS712 sensitivity and apply calibration
    float ampsRMS = ((vrms * 1000.0) / _sensitivity) - _calibrationOffset;
    
    // Ensure non-negative current
    return (ampsRMS < 0) ? 0 : ampsRMS;
}

// Peak-to-peak voltage measurement over specified time period
float CURRENTSensor::getVPP(uint32_t sampleTimeMs) {
    int readValue;
    int maxValue = 0;         // Store max ADC value
    int minValue = 4095;      // Store min ADC value (ESP32 12-bit ADC max)
    
    uint32_t startTime = millis();
    while ((millis() - startTime) < sampleTimeMs) {
        readValue = analogRead(_pin);
        
        // Track maximum and minimum values
        if (readValue > maxValue) {
            maxValue = readValue;
        }
        if (readValue < minValue) {
            minValue = readValue;
        }
        
        // Small delay to prevent overwhelming the ADC
        delayMicroseconds(50);
    }
    
    // Convert ADC difference to voltage
    float result = ((maxValue - minValue) * _vRef) / 4095.0;
    return result;
}

// Calculate power consumption
float CURRENTSensor::calculatePower(float currentRMS, float voltage, float calibrationFactor) {
    // Power = Current × Voltage / Calibration Factor
    // Calibration factor accounts for power factor and measurement errors
    return (currentRMS * voltage) / calibrationFactor;
}
