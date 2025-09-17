
#include <Arduino.h>
#include "PinConfig.h"
#include "DHTSensor.h"
#include "CURRENTSensor.h"
#include "VOLTAGESensor.h"
#include "BUZZERConfig.h"
#include "TRIACModule.h"




// Create DHT sensor object
DHTSensor dhtSensor(DHT_PIN);

// Create Current sensor object (ACS712 5A)
CURRENTSensor currentSensor(CURRENT_SENSOR_PIN);

// Create Voltage sensor object (ZMPT101B)
VOLTAGESensor voltageSensor(VOLTAGE_SENSOR_PIN);

// Simple fan control variables
double temperatureSetpoint = 28.0; // Target temperature in °C
int fanSpeed = 0; // Fan speed (0-100%)

// Buzzer object (Piezo buzzer)
BUZZERConfig buzzer(BUZZER_PIN);

// TRIACModule object for PWM control
TRIACModule triac(TRIAC_PIN, ZERO_CROSS_PIN);

void setup()
{
    Serial.begin(115200);
    
    dhtSensor.begin();
    currentSensor.begin();
    voltageSensor.begin();
    buzzer.begin();

    // TRIACModule setup
    triac.begin();
    
    Serial.println("Smart Fan System Initialized");
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

    // Simple threshold-based fan control
    if (temperature > temperatureSetpoint + 2) {
        fanSpeed = 100; // Full speed if 2°C above setpoint
    } else if (temperature > temperatureSetpoint + 1) {
        fanSpeed = 75;  // 75% speed if 1°C above setpoint
    } else if (temperature > temperatureSetpoint) {
        fanSpeed = 50;  // 50% speed if above setpoint
    } else {
        fanSpeed = 25;  // Low speed if at or below setpoint
    }
    
    // Apply fan speed to TRIAC (you may want to use fanSpeed instead of the sweep test)
    // triac.setPower(fanSpeed); // Uncomment this to use temperature-based control

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
    Serial.print(" kWh, Fan Speed: ");
    Serial.print(fanSpeed);
    Serial.println(" %");

    delay(2000); // Read every 2 seconds
}