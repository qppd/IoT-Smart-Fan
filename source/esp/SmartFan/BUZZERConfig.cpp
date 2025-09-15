#include "BUZZERConfig.h"

BUZZERConfig::BUZZERConfig(uint8_t pin) : _pin(pin) {}

void BUZZERConfig::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void BUZZERConfig::beep(unsigned int duration) {
    digitalWrite(_pin, HIGH);
    delay(duration);
    digitalWrite(_pin, LOW);
}

void BUZZERConfig::alarm(unsigned int times, unsigned int duration, unsigned int pause) {
    for (unsigned int i = 0; i < times; i++) {
        beep(duration);
        delay(pause);
    }
}
