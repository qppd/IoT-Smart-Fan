#ifndef CURRENTSENSOR_H
#define CURRENTSENSOR_H

#include <Arduino.h>

class CURRENTSensor {
public:
    CURRENTSensor(uint8_t pin, float vRef = 3.3, float sensitivity = 0.185);
    void begin();
    float readCurrent(uint16_t samples = 100);
private:
    uint8_t _pin;
    float _vRef;
    float _sensitivity;
};

#endif // CURRENTSENSOR_H
