#include "DHTSensor.h"
#include <DHT.h>

#define DHTTYPE DHT22

DHT dht_global(0, DHTTYPE); // Will be initialized in begin()

DHTSensor::DHTSensor(uint8_t pin) : _pin(pin) {}

void DHTSensor::begin() {
    dht_global = DHT(_pin, DHTTYPE);
    dht_global.begin();
}

float DHTSensor::readTemperature() {
    return dht_global.readTemperature();
}

float DHTSensor::readHumidity() {
    return dht_global.readHumidity();
}