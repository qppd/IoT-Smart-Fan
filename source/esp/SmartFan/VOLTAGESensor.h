#ifndef VOLTAGESENSOR_H
#define VOLTAGESENSOR_H

#include <Arduino.h>

class VOLTAGESensor {
public:
    VOLTAGESensor(uint8_t pin, float vRef = 3.3, float calibration = 1.0);
    void begin();
    float readVoltage(uint16_t samples = 1000);
private:
    uint8_t _pin;
    float _vRef;
    float _calibration;
};

#endif // VOLTAGESENSOR_H
