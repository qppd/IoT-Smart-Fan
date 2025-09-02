#include <Arduino.h>
#include "PinConfig.h"
#include "DHTSensor.h"

#include "FirebaseConfig.h"
#include "CURRENTSensor.h"

#include "VOLTAGESensor.h"


// Create DHT sensor object
DHTSensor dhtSensor(DHT_PIN);

// Create Current sensor object (ACS712 5A on pin 34 by default)
#define CURRENT_SENSOR_PIN 34
CURRENTSensor currentSensor(CURRENT_SENSOR_PIN);

// Create Voltage sensor object (ZMPT101B on pin 35 by default)
#define VOLTAGE_SENSOR_PIN 35
VOLTAGESensor voltageSensor(VOLTAGE_SENSOR_PIN);

void setup()
{
    Serial.begin(115200);
    dhtSensor.begin();
    initFirebase();
    currentSensor.begin();
    voltageSensor.begin();
}

void loop()
{
    float temperature = dhtSensor.readTemperature();
    float humidity = dhtSensor.readHumidity();

    // Microtask: Read current
    float current = currentSensor.readCurrent();

    // Microtask: Read voltage
    float voltage = voltageSensor.readVoltage();


    // Microtask: Print sensor values
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.print(" %, Current: ");
    Serial.print(current, 3);
    Serial.print(" A, Voltage: ");
    Serial.print(voltage, 1);
    Serial.println(" V");


    // Microtask: Send data to Firebase
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

    if (Firebase.RTDB.setFloat(&fbdo, "/current", current)) {
        Serial.println("Current sent to Firebase.");
    } else {
        Serial.print("Failed to send current: ");
        Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/voltage", voltage)) {
        Serial.println("Voltage sent to Firebase.");
    } else {
        Serial.print("Failed to send voltage: ");
        Serial.println(fbdo.errorReason());
    }

    delay(2000); // Read every 2 seconds
}