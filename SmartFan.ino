#include <Arduino.h>
#include "PinConfig.h"
#include "DHTSensor.h"

// Create DHT sensor object
DHTSensor dhtSensor(DHT_PIN);

void setup()
{
    Serial.begin(115200);
    dhtSensor.begin();
}

void loop()
{
    float temperature = dhtSensor.readTemperature();
    float humidity = dhtSensor.readHumidity();

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    delay(2000); // Read every
}