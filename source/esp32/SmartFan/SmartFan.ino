
#include <Arduino.h>
#include "PinConfig.h"
#include "DHTSensor.h"
#include "CURRENTSensor.h"
#include "BUZZERConfig.h"
#include "TRIACModule.h"
#include "ESPCommunication.h"

// Create sensor and control objects
DHTSensor dhtSensor(DHT_PIN);
// Current sensor with calibrated settings based on multimeter comparison
// Calibration: Reading was 0.074A, actual was 0.18A (ratio: 2.43x)
// Adjusted voltage divider factor from 1.5 to 3.65 (1.5 * 2.43) to compensate
CURRENTSensor currentSensor(CURRENT_SENSOR_PIN, 3.3, 185.0, 0.0, 2.43);
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
    // Note: Voltage sensor removed - using fixed 220V value
    buzzer.begin();
    triac.begin();
    
    // ESP Communication setup
    espComm.begin(9600);
    
    Serial.println("Smart Fan ESP32 Initialized Successfully!");
    
    // Test current sensor calibration (enabled for calibration verification)
    testCurrentSensorCalibration();
    
    // Test communication (comment out after testing)
    delay(2000); // Wait for ESP8266 to initialize
    testESP32Communication();
    
    // Test TRIAC dimmer (uncomment for testing)
    // testTRIACDimmer();
    
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
    
    // Use new RMS current measurement for better AC accuracy
    sensors.current = currentSensor.readCurrentRMS(1000); // 1 second sampling
    sensors.voltage = 240.0; // Fixed voltage value (adjust for your local AC voltage)
    
    // Validate sensor readings
    sensors.sensorsOk = (!isnan(sensors.temperature) && 
                        !isnan(sensors.humidity) && 
                        sensors.current >= 0
                        );
    
    if (sensors.sensorsOk) {
        // Calculate power using improved power calculation method
        sensors.watt = currentSensor.calculatePower(sensors.current, sensors.voltage, 1.2);
        
        // Calculate energy consumption (kWh) with improved accuracy
        if (lastUpdateMillis > 0) {
            float hours = (millis() - lastUpdateMillis) / 3600000.0;
            sensors.kwh += (sensors.watt * hours) / 1000.0;
        }
        lastUpdateMillis = millis();
        
        // Add power monitoring alerts
        checkPowerLimits();
    } else {
        Serial.println("⚠️ Sensor reading error detected");
        // Set safe defaults for failed readings
        sensors.current = 0.0;
        sensors.watt = 0.0;
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
    
    Serial.println("\n=== ESP32 Smart Fan System Status ===");
    Serial.printf("🌡️ Temperature: %.1f°C (Target: %.1f°C)\n", 
                  sensors.temperature, fanControl.targetTemperature);
    Serial.printf("💧 Humidity: %.1f%%\n", sensors.humidity);
    Serial.printf("⚡ Voltage: %.1fV, Current: %.3fA (RMS)\n", sensors.voltage, sensors.current);
    Serial.printf("🔌 Power: %.2fW, Energy: %.4f kWh\n", sensors.watt, sensors.kwh);
    Serial.printf("🌀 Fan Speed: %d%% (Target: %d%%)\n", 
                  fanControl.currentFanSpeed, fanControl.targetFanSpeed);
    
    // Show power efficiency if fan is running
    if (fanControl.currentFanSpeed > 0) {
        float efficiency = sensors.watt / fanControl.currentFanSpeed;
        Serial.printf("📊 Power Efficiency: %.2f W/%% fan speed\n", efficiency);
    }
    
    Serial.printf("🔧 Sensors: %s, Auto Mode: %s, Buzzer: %s\n", 
                  sensors.sensorsOk ? "OK" : "ERROR", 
                  fanControl.autoMode ? "ON" : "OFF",
                  fanControl.buzzerEnabled ? "ON" : "OFF");
    Serial.println("==========================================\n");
    
    lastStatusPrint = millis();
}

// Power monitoring and safety checks
void checkPowerLimits() {
    // Define power thresholds
    const float MAX_POWER_WATTS = 150.0;        // Maximum safe power consumption
    const float HIGH_POWER_WATTS = 100.0;       // High power warning threshold
    const float MAX_CURRENT_AMPS = 0.7;         // Maximum safe current (for 5A sensor)
    
    // Check for overcurrent conditions
    if (sensors.current > MAX_CURRENT_AMPS) {
        Serial.println("🚨 OVERCURRENT DETECTED! Reducing fan speed for safety.");
        fanControl.targetFanSpeed = min(fanControl.targetFanSpeed, 50); // Limit to 50%
        fanControl.buzzerAlert = true;
    }
    
    // Check for high power consumption
    if (sensors.watt > MAX_POWER_WATTS) {
        Serial.printf("🚨 HIGH POWER CONSUMPTION: %.1fW (Max: %.1fW)\n", sensors.watt, MAX_POWER_WATTS);
        fanControl.targetFanSpeed = min(fanControl.targetFanSpeed, 75); // Limit to 75%
        fanControl.buzzerAlert = true;
    } else if (sensors.watt > HIGH_POWER_WATTS) {
        Serial.printf("⚠️ Power warning: %.1fW (Warning: %.1fW)\n", sensors.watt, HIGH_POWER_WATTS);
    }
    
    // Log power efficiency information
    static unsigned long lastEfficiencyLog = 0;
    if (millis() - lastEfficiencyLog > 30000) { // Log every 30 seconds
        float efficiency = (fanControl.currentFanSpeed > 0) ? sensors.watt / fanControl.currentFanSpeed : 0;
        Serial.printf("💡 Power Efficiency: %.2f W/% fan speed\n", efficiency);
        lastEfficiencyLog = millis();
    }
}

// Test function for current sensor calibration
void testCurrentSensorCalibration() {
    Serial.println("=== Current Sensor Calibration Test ===");
    Serial.println("Testing ACS712 with voltage divider calibration...");
    
    // Test both measurement methods
    for (int i = 0; i < 5; i++) {
        Serial.printf("Test %d:\n", i + 1);
        
        // Original method
        float currentOld = currentSensor.readCurrent(200);
        
        // New RMS method
        float currentRMS = currentSensor.readCurrentRMS(1000);
        
        // Peak-to-peak voltage
        float vpp = currentSensor.getVPP(1000);
        
        Serial.printf("  Original Method: %.3f A\n", currentOld);
        Serial.printf("  RMS Method: %.3f A\n", currentRMS);
        Serial.printf("  Peak-to-Peak Voltage: %.3f V\n", vpp);
        
        // Calculate power with new method
        float power = currentSensor.calculatePower(currentRMS, 240.0, 1.2);
        Serial.printf("  Calculated Power: %.2f W\n", power);
        Serial.println();
        
        delay(2000);
    }
    
    Serial.println("=== Current Sensor Calibration Test Complete ===\n");
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

// Test function for TRIAC dimmer module
void testTRIACDimmer() {
    Serial.println("=== TRIAC Dimmer Test ===");
    Serial.println("Testing RobotDyn Dimmer Library functionality...");
    
    // Test power levels from 0% to 100%
    for (int power = 0; power <= 100; power += 25) {
        Serial.print("Setting TRIAC power to: ");
        Serial.print(power);
        Serial.println("%");
        
        triac.setPower(power);
        
        Serial.print("Actual TRIAC power: ");
        Serial.print(triac.getPower());
        Serial.println("%");
        
        delay(2000); // Hold each power level for 2 seconds
    }
    
    // Return to 0% power
    triac.setPower(0);
    Serial.println("TRIAC power returned to 0%");
    Serial.println("=== TRIAC Dimmer Test Complete ===");
}
