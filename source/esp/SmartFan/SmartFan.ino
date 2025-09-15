
#include <Arduino.h>
#include "PinConfig.h"
#include "DHTSensor.h"
#include "FirebaseConfig.h"
#include "CURRENTSensor.h"
#include "VOLTAGESensor.h"
#include "PIDConfig.h"
#include "BUZZERConfig.h"
#include "TRIACModule.h"




// Create DHT sensor object
DHTSensor dhtSensor(DHT_PIN);

// Create Firebase manager object
FirebaseManager firebaseManager;

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

// TRIACModule object for PWM control (example: pin 12)
TRIACModule triac(12);

void setup()
{
    Serial.begin(115200);
    dhtSensor.begin();
    firebaseManager.begin();
    firebaseManager.beginTokenStream();
    currentSensor.begin();
    voltageSensor.begin();
    fanPID.begin();
    fanPID.setOutputLimits(0, 255); // For PWM control (0-255)
    buzzer.begin();

    // TRIACModule setup
    triac.begin();
}

void loop()
{

    // Microtask: Read sensors
    float temperature = dhtSensor.readTemperature();
    float humidity = dhtSensor.readHumidity();
    float current = currentSensor.readCurrent();
    float voltage = voltageSensor.readVoltage();
    float watt = voltage * current;
    static float kwh = 0;
    static unsigned long lastUpdateMillis = 0;
    unsigned long nowMillis = millis();
    // kWh calculation: energy (Wh) = power (W) * time (h)
    if (lastUpdateMillis > 0) {
        float hours = (nowMillis - lastUpdateMillis) / 3600000.0;
        kwh += watt * hours / 1000.0; // kWh
    }
    lastUpdateMillis = nowMillis;

    // TRIACModule test: sweep power from 0% to 100% and back
    static int power = 0;
    static int dir = 1;
    triac.setPower(power);
    Serial.print("TRIAC Power: ");
    Serial.print(power);
    Serial.println(" %");
    delay(50);
    power += dir;
    if (power >= 100) dir = -1;
    if (power <= 0) dir = 1;

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
    Serial.print(" V, Power: ");
    Serial.print(watt, 2);
    Serial.print(" W, Energy: ");
    Serial.print(kwh, 5);
    Serial.print(" kWh, Fan PWM: ");
    Serial.println((int)fanOutput);


    // Microtask: Send device state and log to Firebase (new structure)
    String deviceId = "deviceIdABC"; // TODO: Set this per device
    String mode = "auto"; // Example, set as needed
    unsigned long now = millis() / 1000 + 1692620000; // Example timestamp logic
    firebaseManager.updateDeviceCurrent(deviceId, temperature, (int)fanOutput, mode, now, voltage, current, watt, kwh);
    firebaseManager.logDeviceData(deviceId, now, temperature, (int)fanOutput, voltage, current, watt, kwh);

    delay(2000); // Read every 2 seconds
}