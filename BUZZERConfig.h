#ifndef BUZZERCONFIG_H
#define BUZZERCONFIG_H

#include <Arduino.h>

class BUZZERConfig {
public:
    BUZZERConfig(uint8_t pin);
    void begin();
    void beep(unsigned int duration = 200);
    void alarm(unsigned int times = 3, unsigned int duration = 200, unsigned int pause = 100);
private:
    uint8_t _pin;
};

#endif // BUZZERCONFIG_H
