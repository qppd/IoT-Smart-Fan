
#include <Arduino.h>
#include "PinConfig.h"
#include "DHTSensor.h"
#include "CURRENTSensor.h"
#include "VOLTAGESensor.h"
#include "BUZZERConfig.h"
#include "TRIACModule.h"
#include "ESPCommunication.h"

// Create sensor and control objects
DHTSensor dhtSensor(DHT_PIN);
CURRENTSensor currentSensor(CURRENT_SENSOR_PIN);
VOLTAGESensor voltageSensor(VOLTAGE_SENSOR_PIN);
BUZZERConfig buzzer(BUZZER_PIN);
TRIACModule triac(TRIAC_PIN, ZERO_CROSS_PIN);
ESPCommunication espComm(ESP_SERIAL_RX, ESP_SERIAL_TX);

// Control variables - updated from ESP8266 commands
struct FanControl {
    float targetTemperature = 28.0;    // Default target temperature
    int currentFanSpeed = 0;           // Current fan speed (0-100%)
    int targetFanSpeed = 50;           // Target fan speed from ESP8266
    bool autoMode = true;              // Automatic temperature control
    bool buzzerEnabled = true;         // Buzzer alerts enabled
    bool buzzerAlert = false;          // Trigger buzzer alert
};

FanControl fanControl;

// Sensor data structure
struct SensorReadings {
    float temperature = 25.0;
    float humidity = 50.0;
    float voltage = 220.0;
    float current = 0.5;
    float watt = 0.0;
    float kwh = 0.0;
    bool sensorsOk = false;
};

SensorReadings sensors;

unsigned long lastSensorRead = 0;
unsigned long lastCommSend = 0;
unsigned long lastUpdateMillis = 0;
const unsigned long SENSOR_READ_INTERVAL = 2000;   // Read sensors every 2 seconds
const unsigned long COMM_SEND_INTERVAL = 5000;     // Send data to ESP8266 every 5 seconds

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Smart Fan ESP32 - Sensor & Hardware Controller ===");
    
    // Initialize all components
    dhtSensor.begin();
    currentSensor.begin();
    voltageSensor.begin();
    buzzer.begin();
    triac.begin();
    
    // ESP Communication setup
    espComm.begin(9600);
    
    Serial.println("Smart Fan ESP32 Initialized Successfully!");
    
    // Test communication (comment out after testing)
    delay(2000); // Wait for ESP8266 to initialize
    testESP32Communication();
    
    Serial.println("Starting main sensor and control loop...");
}

void loop() {
    // Read sensors periodically
    if (millis() - lastSensorRead > SENSOR_READ_INTERVAL) {
        readAllSensors();
        lastSensorRead = millis();
    }
    
    // Process commands from ESP8266
    espComm.processIncomingData();
    updateControlFromESP8266();
    
    // Control fan based on temperature or ESP8266 commands
    controlFanSpeed();
    
    // Handle buzzer alerts
    handleBuzzerAlerts();
    
    // Send sensor data to ESP8266 periodically
    if (millis() - lastCommSend > COMM_SEND_INTERVAL) {
        sendDataToESP8266();
        lastCommSend = millis();
    }
    
    // Print system status
    printSystemStatus();
    
    delay(500); // Main loop delay
}

void readAllSensors() {
    // Read all sensors
    sensors.temperature = dhtSensor.readTemperature();
    sensors.humidity = dhtSensor.readHumidity();
    sensors.current = currentSensor.readCurrent();
    sensors.voltage = voltageSensor.readVoltage();
    
    // Validate sensor readings
    sensors.sensorsOk = (!isnan(sensors.temperature) && 
                        !isnan(sensors.humidity) && 
                        sensors.voltage > 0 && 
                        sensors.current >= 0);
    
    if (sensors.sensorsOk) {
        // Calculate power
        sensors.watt = sensors.voltage * sensors.current;
        
        // Calculate energy consumption (kWh)
        if (lastUpdateMillis > 0) {
            float hours = (millis() - lastUpdateMillis) / 3600000.0;
            sensors.kwh += (sensors.watt * hours) / 1000.0;
        }
        lastUpdateMillis = millis();
    } else {
        Serial.println("⚠️ Sensor reading error detected");
    }
}

void updateControlFromESP8266() {
    ControlSettings settings = espComm.getControlSettings();
    
    // Update control parameters if we have fresh commands
    if (espComm.hasNewCommands()) {
        fanControl.targetFanSpeed = settings.targetFanSpeed;
        fanControl.targetTemperature = settings.targetTemperature;
        
        Serial.println("📡 Updated from ESP8266: Fan=" + String(fanControl.targetFanSpeed) + 
                      "%, Temp=" + String(fanControl.targetTemperature, 1) + "°C");
    }
}

void controlFanSpeed() {
    int newFanSpeed = fanControl.currentFanSpeed;
    
    if (fanControl.autoMode && sensors.sensorsOk) {
        // Automatic temperature-based control
        float tempDiff = sensors.temperature - fanControl.targetTemperature;
        
        if (tempDiff > 3.0) {
            newFanSpeed = 100;  // Full speed if 3°C+ above target
        } else if (tempDiff > 2.0) {
            newFanSpeed = 80;   // 80% speed if 2°C+ above target  
        } else if (tempDiff > 1.0) {
            newFanSpeed = 60;   // 60% speed if 1°C+ above target
        } else if (tempDiff > 0.5) {
            newFanSpeed = 40;   // 40% speed if 0.5°C+ above target
        } else if (tempDiff > 0) {
            newFanSpeed = 25;   // Low speed if slightly above target
        } else {
            newFanSpeed = 10;   // Minimal speed if at or below target
        }
        
        // Also consider manual override from ESP8266
        newFanSpeed = max(newFanSpeed, fanControl.targetFanSpeed);
        
    } else {
        // Manual mode - use ESP8266 commanded speed
        newFanSpeed = fanControl.targetFanSpeed;
    }
    
    // Apply speed limits and update TRIAC
    newFanSpeed = constrain(newFanSpeed, 0, 100);
    
    if (newFanSpeed != fanControl.currentFanSpeed) {
        fanControl.currentFanSpeed = newFanSpeed;
        triac.setPower(fanControl.currentFanSpeed);
        Serial.println("🌀 Fan speed updated: " + String(fanControl.currentFanSpeed) + "%");
    }
}

void handleBuzzerAlerts() {
    bool shouldAlert = false;
    
    // Temperature alert
    if (sensors.sensorsOk && sensors.temperature > (fanControl.targetTemperature + 3.0)) {
        shouldAlert = true;
    }
    
    // Manual alert from ESP8266
    if (fanControl.buzzerAlert) {
        shouldAlert = true;
        fanControl.buzzerAlert = false; // Reset flag
    }
    
    // Activate buzzer if needed
    if (shouldAlert && fanControl.buzzerEnabled) {
        buzzer.beep(300); // Beep for 300ms
        espComm.sendBuzzerStatus(true);
        Serial.println("🔔 Buzzer alert triggered!");
    } else {
        espComm.sendBuzzerStatus(false);
    }
}

void sendDataToESP8266() {
    if (!sensors.sensorsOk) {
        espComm.sendStatus("SENSOR_ERROR");
        return;
    }
    
    // Send all sensor data in one message
    espComm.sendAllSensorData(
        sensors.temperature,
        sensors.humidity, 
        sensors.voltage,
        sensors.current,
        fanControl.currentFanSpeed
    );
    
    // Send status
    espComm.sendStatus("RUNNING");
    
    Serial.println("📡 Sensor data sent to ESP8266");
}

void printSystemStatus() {
    static unsigned long lastStatusPrint = 0;
    if (millis() - lastStatusPrint < 10000) return; // Print every 10 seconds
    
    Serial.println("\n=== ESP32 System Status ===");
    Serial.printf("Temperature: %.1f°C (Target: %.1f°C)\n", 
                  sensors.temperature, fanControl.targetTemperature);
    Serial.printf("Humidity: %.1f%%\n", sensors.humidity);
    Serial.printf("Voltage: %.1fV, Current: %.3fA\n", sensors.voltage, sensors.current);
    Serial.printf("Power: %.2fW, Energy: %.4f kWh\n", sensors.watt, sensors.kwh);
    Serial.printf("Fan Speed: %d%% (Target: %d%%)\n", 
                  fanControl.currentFanSpeed, fanControl.targetFanSpeed);
    Serial.printf("Sensors: %s, Auto Mode: %s\n", 
                  sensors.sensorsOk ? "OK" : "ERROR", 
                  fanControl.autoMode ? "ON" : "OFF");
    Serial.println("=========================\n");
    
    lastStatusPrint = millis();
}

// Test function for ESP32 communication
void testESP32Communication() {
    Serial.println("=== ESP32 Communication Test ===");
    
    bool testResult = espComm.testCommunication();
    
    if (testResult) {
        Serial.println("✅ ESP8266 communication working!");
    } else {
        Serial.println("❌ ESP8266 communication failed!");
    }
    
    Serial.println("=== ESP32 Communication Test Complete ===");
}
