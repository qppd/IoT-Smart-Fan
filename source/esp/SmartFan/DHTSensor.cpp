#include "DHTSensor.h"
#include <DHT.h>

#define DHTTYPE DHT22

DHTSensor::DHTSensor(uint8_t pin) : _pin(pin) {}

void DHTSensor::begin() {
    static DHT dht(_pin, DHTTYPE);
    dht.begin();
}

float DHTSensor::readTemperature() {
    static DHT dht(_pin, DHTTYPE);
    return dht.readTemperature();
}

float DHTSensor::readHumidity() {
    static DHT dht(_pin, DHTTYPE);
    return