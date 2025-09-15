#include "TRIACModule.h"

TRIACModule::TRIACModule(uint8_t pwmPin) : _pwmPin(pwmPin), _currentPower(0) {}

void TRIACModule::begin() {
    pinMode(_pwmPin, OUTPUT);
    setPower(0);
}

void TRIACModule::setPower(uint8_t percent) {
    if (percent > 100) percent = 100;
    _currentPower = percent;
    // Map 0-100% to 0-255 PWM
    uint8_t pwmValue = map(percent, 0, 100, 0, 255);
    analogWrite(_pwmPin, pwmValue);
}
