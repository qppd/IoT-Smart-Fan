#include <Arduino.h>
#include "PinConfig.h"
#include "DHTSensor.h"

#include "FirebaseConfig.h"
#include "CURRENTSensor.h"

#include "VOLTAGESensor.h"

#include "PIDConfig.h"

#include "BUZZERConfig.h"


// Create DHT sensor object
DHTSensor dhtSensor(DHT_PIN);

// Create Current sensor object (ACS712 5A on pin 34 by default)
#define CURRENT_SENSOR_PIN 34
CURRENTSensor currentSensor(CURRENT_SENSOR_PIN);

// Create Voltage sensor object (ZMPT101B on pin 35 by default)
#define VOLTAGE_SENSOR_PIN 35
VOLTAGESensor voltageSensor(VOLTAGE_SENSOR_PIN);

// PID control variables
double temperatureInput = 0, fanOutput = 0, temperatureSetpoint = 28.0; // Example setpoint 28°C
PIDConfig fanPID(&temperatureInput, &fanOutput, &temperatureSetpoint, 2.0, 5.0, 1.0); // Example PID values

// Buzzer object (Piezo buzzer on pin 25 by default)
#define BUZZER_PIN 25
BUZZERConfig buzzer(BUZZER_PIN);

void setup()
{
    Serial.begin(115200);
    dhtSensor.begin();
    initFirebase();
    currentSensor.begin();
    voltageSensor.begin();
    fanPID.begin();
    fanPID.setOutputLimits(0, 255); // For PWM control (0-255)
    buzzer.begin();
}

void loop()
{

    // Microtask: Read sensors
    float temperature = dhtSensor.readTemperature();
    float humidity = dhtSensor.readHumidity();
    float current = currentSensor.readCurrent();
    float voltage = voltageSensor.readVoltage();

    // Microtask: PID control for fan speed
    temperatureInput = temperature;
    fanPID.compute();

    // Microtask: Buzzer alert if temperature exceeds threshold
    if (temperature > (temperatureSetpoint + 2)) {
        buzzer.beep(300); // Beep for 300ms if temperature is too high
    }


    // Microtask: Print sensor and control values
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.print(" %, Current: ");
    Serial.print(current, 3);
    Serial.print(" A, Voltage: ");
    Serial.print(voltage, 1);
    Serial.print(" V, Fan PWM: ");
    Serial.println((int)fanOutput);


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

    if (Firebase.RTDB.setFloat(&fbdo, "/fan_pwm", fanOutput)) {
        Serial.println("Fan PWM sent to Firebase.");
    } else {
        Serial.print("Failed to send fan PWM: ");
        Serial.println(fbdo.errorReason());
    }

    delay(2000); // Read every 2 seconds
}