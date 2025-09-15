#include "VOLTAGESensor.h"

VOLTAGESensor::VOLTAGESensor(uint8_t pin, float vRef, float calibration)
    : _pin(pin), _vRef(vRef), _calibration(calibration) {}

void VOLTAGESensor::begin() {
    pinMode(_pin, INPUT);
}

float VOLTAGESensor::readVoltage(uint16_t samples) {
    float maxValue = 0;
    float minValue = 4095;
    float sum = 0;
    for (uint16_t i = 0; i < samples; i++) {
        int adc = analogRead(_pin);
        sum += adc;
        if (adc > maxValue) maxValue = adc;
        if (adc < minValue) minValue = adc;
        delayMicroseconds(100);
    }
    float avg = sum / samples;
    float voltage = (avg / 4095.0) * _vRef;
    // For ZMPT101B, you typically need to calibrate for true RMS and scale to mains voltage
    // This is a simplified version; for accurate results, calibrate with a true voltmeter
    float mainsVoltage = voltage * _calibration * 100.0; // Example scaling
    return mainsVoltage;
}
