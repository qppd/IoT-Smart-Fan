#ifndef TRIACMODULE_H
#define TRIACMODULE_H

#include <Arduino.h>

class TRIACModule {
public:
    TRIACModule(uint8_t pwmPin);
    void begin();
    void setPower(uint8_t percent); // percent: 0-100
private:
    uint8_t _pwmPin;
    uint8_t _currentPower;
};

#endif // TRIACMODULE_H
