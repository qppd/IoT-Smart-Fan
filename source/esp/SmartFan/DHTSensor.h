#ifndef DHTSENSOR_H
#define DHTSENSOR_H

#include <Arduino.h>

class DHTSensor {
public:
  DHTSensor(uint8_t pin);
  void begin();
  float readTemperature();
  float readHumidity();

private:
  uint8_t _pin;
};

#endif  // DHTSENSOR_H