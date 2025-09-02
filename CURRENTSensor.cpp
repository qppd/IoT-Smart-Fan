#include "CURRENTSensor.h"

CURRENTSensor::CURRENTSensor(uint8_t pin, float vRef, float sensitivity)
    : _pin(pin), _vRef(vRef), _sensitivity(sensitivity) {}

void CURRENTSensor::begin() {
    pinMode(_pin, INPUT);
}

float CURRENTSensor::readCurrent(uint16_t samples) {
    float avg = 0;
    for (uint16_t i = 0; i < samples; i++) {
        avg += analogRead(_pin);
        delayMicroseconds(100);
    }
    avg /= samples;
    float voltage = (avg / 4095.0) * _vRef; // ESP32 ADC is 12-bit (0-4095)
    float offset = _vRef / 2.0; // ACS712 output is Vcc/2 at 0A
    float current = (voltage - offset) / _sensitivity;
    return current;
}
