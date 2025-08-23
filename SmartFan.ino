#include <Arduino.h>
#include "PinConfig.h"
#include "DHTSensor.h"
#include "FirebaseConfig.h"

// Create DHT sensor object
DHTSensor dhtSensor(DHT_PIN);

void setup()
{
    Serial.begin(115200);
    dhtSensor.begin();
    initFirebase();
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

    if (Firebase.RTDB.setFloat(&fbdo, "/temperature", temperature)) {
        Serial.println("Temperature sent to Firebase.");
    } else {
        Serial.print("Failed to send temperature: ");
        Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/humidity", humidity)) {
        Serial.println("Humidity sent to Firebase.");
    } else {
        Serial.print("Failed to send humidity: ");
        Serial.println(fbdo.errorReason());
    }

    delay(2000); // Read every
}