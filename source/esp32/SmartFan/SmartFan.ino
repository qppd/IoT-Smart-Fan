
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
  float targetTemperature = 28.0;  // Default target temperature
  int currentFanSpeed = 0;         // Current fan speed (0-100%)
  int targetFanSpeed = 50;         // Target fan speed from ESP8266
  bool autoMode = true;            // Automatic temperature control
  bool buzzerEnabled = true;       // Buzzer alerts enabled
  bool buzzerAlert = false;        // Trigger buzzer alert
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

// Temperature testing variables
bool temperatureTestMode = false;
float testTemperature = 25.0;

unsigned long lastSensorRead = 0;
unsigned long lastCommSend = 0;
unsigned long lastUpdateMillis = 0;
const unsigned long SENSOR_READ_INTERVAL = 2000;  // Read sensors every 2 seconds
const unsigned long COMM_SEND_INTERVAL = 5000;    // Send data to ESP8266 every 5 seconds

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
  delay(2000);  // Wait for ESP8266 to initialize
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

  // Process serial commands for testing
  processSerialCommands();

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

  delay(500);  // Main loop delay
}

void readAllSensors() {
  // Read all sensors
  if (temperatureTestMode) {
    sensors.temperature = testTemperature;
    Serial.printf("🧪 TEST MODE - Using manual temperature: %.1f°C\n", sensors.temperature);
  } else {
    sensors.temperature = dhtSensor.readTemperature();
  }
  sensors.humidity = dhtSensor.readHumidity();

  // Use new RMS current measurement for better AC accuracy
  sensors.current = currentSensor.readCurrentRMS(1000);  // 1 second sampling
  sensors.voltage = 240.0;                               // Fixed voltage value (adjust for your local AC voltage)

  // Validate sensor readings
  sensors.sensorsOk = (!isnan(sensors.temperature) && !isnan(sensors.humidity) && sensors.current >= 0);

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
    fanControl.autoMode = settings.autoMode;  // Update mode from ESP8266

    Serial.println("📡 Updated from ESP8266: Fan=" + String(fanControl.targetFanSpeed) + "%, Temp=" + String(fanControl.targetTemperature, 1) + "°C, Mode=" + String(fanControl.autoMode ? "auto" : "manual"));
  }
}

void controlFanSpeed() {
  int newFanSpeed = fanControl.currentFanSpeed;

  if (fanControl.autoMode && sensors.sensorsOk) {
    // Automatic temperature-based control with absolute temperature thresholds
    float currentTemp = sensors.temperature;

    Serial.printf("🔍 AUTO MODE DEBUG - Current Temp: %.1f°C\n", currentTemp);

    if (currentTemp >= 32.0) {
      newFanSpeed = 99;  // 99% speed if 32°C or above
      Serial.println("🔥 Setting 99% - ≥32°C zone");
    } else if (currentTemp >= 30.0) {
      newFanSpeed = 90;  // 95% speed if 30-31.99°C
      Serial.println("🔥 Setting 90% - 30-31°C zone");
    } else if (currentTemp >= 28.0) {
      newFanSpeed = 80;
      Serial.println("🔥 Setting 80% - 28-29°C zone");
    } else if (currentTemp >= 26.0) {
      newFanSpeed = 70;
      Serial.println("🔥 Setting 70% - 26-27°C zone");
    } else if (currentTemp >= 24.0) {
      newFanSpeed = 60;
      Serial.println("🔥 Setting 60% - 24-25°C zone");
    } else if (currentTemp >= 22.0) {
      newFanSpeed = 50;
      Serial.println("🔥 Setting 50% - 22-23°C zone");
    } else {
      newFanSpeed = 0;  // OFF if below 25°C
      Serial.println("❄️ Setting 0% - <25°C zone (OFF)");
    }

    Serial.printf("📋 Before ESP8266 override - Auto speed: %d%%, ESP8266 target: %d%%\n",
                  newFanSpeed, fanControl.targetFanSpeed);

    // Also consider manual override from ESP8266
    newFanSpeed = max(newFanSpeed, fanControl.targetFanSpeed);
  } else {
    // Manual mode - use ESP8266 commanded speed
    newFanSpeed = fanControl.targetFanSpeed;
  }

  // Apply speed limits and update TRIAC
  newFanSpeed = constrain(newFanSpeed, 0, 100);

  Serial.printf("🎯 FINAL DECISION - Old speed: %d%%, New speed: %d%%\n",
                fanControl.currentFanSpeed, newFanSpeed);

  if (newFanSpeed != fanControl.currentFanSpeed) {
    fanControl.currentFanSpeed = newFanSpeed;
    Serial.printf("🔧 Sending to TRIAC: %d%%\n", fanControl.currentFanSpeed);
    triac.setPower(fanControl.currentFanSpeed);
    Serial.println("🌀 Fan speed updated: " + String(fanControl.currentFanSpeed) + "%");

    // Verify TRIAC received the command
    int triacPower = triac.getPower();
    Serial.printf("✅ TRIAC confirmed power: %d%%\n", triacPower);

    if (triacPower != fanControl.currentFanSpeed) {
      Serial.printf("❌ TRIAC MISMATCH! Expected: %d%%, Actual: %d%%\n",
                    fanControl.currentFanSpeed, triacPower);
    }
  } else {
    Serial.println("⚪ Fan speed unchanged: " + String(fanControl.currentFanSpeed) + "%");
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
    fanControl.buzzerAlert = false;  // Reset flag
  }

  // Activate buzzer if needed
  if (shouldAlert && fanControl.buzzerEnabled) {
    buzzer.beep(300);  // Beep for 300ms
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
    fanControl.currentFanSpeed);

  // Send status
  espComm.sendStatus("RUNNING");

  Serial.println("📡 Sensor data sent to ESP8266");
}

void printSystemStatus() {
  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint < 10000) return;  // Print every 10 seconds

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
  const float MAX_POWER_WATTS = 150.0;   // Maximum safe power consumption
  const float HIGH_POWER_WATTS = 100.0;  // High power warning threshold
  const float MAX_CURRENT_AMPS = 0.7;    // Maximum safe current (for 5A sensor)

  // Check for overcurrent conditions
  if (sensors.current > MAX_CURRENT_AMPS) {
    Serial.println("🚨 OVERCURRENT DETECTED! Reducing fan speed for safety.");
    fanControl.targetFanSpeed = min(fanControl.targetFanSpeed, 50);  // Limit to 50%
    fanControl.buzzerAlert = true;
  }

  // Check for high power consumption
  if (sensors.watt > MAX_POWER_WATTS) {
    Serial.printf("🚨 HIGH POWER CONSUMPTION: %.1fW (Max: %.1fW)\n", sensors.watt, MAX_POWER_WATTS);
    fanControl.targetFanSpeed = min(fanControl.targetFanSpeed, 75);  // Limit to 75%
    fanControl.buzzerAlert = true;
  } else if (sensors.watt > HIGH_POWER_WATTS) {
    Serial.printf("⚠️ Power warning: %.1fW (Warning: %.1fW)\n", sensors.watt, HIGH_POWER_WATTS);
  }

  // Log power efficiency information
  static unsigned long lastEfficiencyLog = 0;
  if (millis() - lastEfficiencyLog > 30000) {  // Log every 30 seconds
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

    delay(2000);  // Hold each power level for 2 seconds
  }

  // Return to 0% power
  triac.setPower(0);
  Serial.println("TRIAC power returned to 0%");
  Serial.println("=== TRIAC Dimmer Test Complete ===");
}

// Serial command processing for testing
void processSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();

    Serial.println("\n🔧 Processing command: " + command);

    if (command.startsWith("TEMP ")) {
      // Set test temperature - Format: "TEMP 32.5"
      float temp = command.substring(5).toFloat();
      if (temp >= 0 && temp <= 60) {
        testTemperature = temp;
        temperatureTestMode = true;
        Serial.printf("✅ Test temperature set to: %.1f°C\n", testTemperature);
        Serial.println("🧪 Temperature test mode ENABLED");
      } else {
        Serial.println("❌ Invalid temperature range (0-60°C)");
      }
    } else if (command == "TEMP OFF") {
      // Disable test mode
      temperatureTestMode = false;
      Serial.println("✅ Temperature test mode DISABLED - using real sensor");
    } else if (command == "TEMP REAL") {
      // Same as TEMP OFF
      temperatureTestMode = false;
      Serial.println("✅ Using real temperature sensor");
    } else if (command.startsWith("FAN ")) {
      // Set manual fan speed - Format: "FAN 75"
      int speed = command.substring(4).toInt();
      if (speed >= 0 && speed <= 100) {
        fanControl.targetFanSpeed = speed;
        fanControl.autoMode = false;
        Serial.printf("✅ Manual fan speed set to: %d%%\n", speed);
        Serial.println("🔧 Auto mode DISABLED");
      } else {
        Serial.println("❌ Invalid fan speed range (0-100%)");
      }
    } else if (command == "AUTO ON") {
      // Enable auto mode
      fanControl.autoMode = true;
      Serial.println("✅ Auto mode ENABLED");
    } else if (command == "AUTO OFF") {
      // Disable auto mode
      fanControl.autoMode = false;
      Serial.println("✅ Auto mode DISABLED");
    } else if (command == "STATUS") {
      // Show current status
      Serial.println("\n=== Manual Testing Status ===");
      Serial.printf("🌡️ Temperature Test Mode: %s\n", temperatureTestMode ? "ON" : "OFF");
      if (temperatureTestMode) {
        Serial.printf("🧪 Test Temperature: %.1f°C\n", testTemperature);
      }
      Serial.printf("🔧 Auto Mode: %s\n", fanControl.autoMode ? "ON" : "OFF");
      Serial.printf("🌀 Manual Fan Speed: %d%%\n", fanControl.targetFanSpeed);
      Serial.printf("📊 Current Fan Speed: %d%%\n", fanControl.currentFanSpeed);
      Serial.printf("🎯 Target Temperature: %.1f°C\n", fanControl.targetTemperature);
      Serial.println("===============================");
    } else if (command == "TEST TEMP") {
      // Run comprehensive temperature control logic test
      testTemperatureControlLogic();
    } else if (command == "TEST TRIAC") {
      // Run comprehensive TRIAC control test
      testTRIACControlLogic();
    } else if (command == "TEST ALL") {
      // Run all tests
      testTemperatureControlLogic();
      delay(2000);
      testTRIACControlLogic();
    } else if (command == "TEST QUICK") {
      // Quick test of key temperature points
      testQuickTemperaturePoints();
    } else if (command == "HELP") {
      // Show help
      Serial.println("\n=== TRIAC Testing Commands ===");
      Serial.println("TEMP <value>     - Set test temperature (0-60°C)");
      Serial.println("TEMP OFF         - Use real temperature sensor");
      Serial.println("TEMP REAL        - Use real temperature sensor");
      Serial.println("FAN <value>      - Set manual fan speed (0-100%) + disable auto");
      Serial.println("AUTO ON          - Enable automatic temperature control");
      Serial.println("AUTO OFF         - Disable automatic temperature control");
      Serial.println("STATUS           - Show current testing status");
      Serial.println("TEST TEMP        - Run comprehensive temperature control test");
      Serial.println("TEST TRIAC       - Run comprehensive TRIAC control test");
      Serial.println("TEST ALL         - Run all comprehensive tests");
      Serial.println("TEST QUICK       - Quick test of key temperature points");
      Serial.println("HELP             - Show this help menu");
      Serial.println("\nExamples:");
      Serial.println("  TEMP 35        - Test with 35°C (should trigger 100% fan)");
      Serial.println("  TEMP 30        - Test with 30°C (should trigger 80% fan)");
      Serial.println("  FAN 25         - Set fan to 25% manually");
      Serial.println("  AUTO ON        - Return to temperature-based control");
      Serial.println("================================");
    } else {
      Serial.println("❌ Unknown command. Type 'HELP' for available commands.");
    }

    Serial.println();  // Extra line for readability
  }
}

// Comprehensive Temperature Control Logic Test
void testTemperatureControlLogic() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║    TEMPERATURE CONTROL LOGIC TEST     ║");
  Serial.println("╚════════════════════════════════════════╝");

  // Save current state
  bool originalTestMode = temperatureTestMode;
  bool originalAutoMode = fanControl.autoMode;
  float originalTemp = testTemperature;

  // Enable test mode and auto mode
  temperatureTestMode = true;
  fanControl.autoMode = true;

  // Test temperature points and expected fan speeds
  struct TempTest {
    float temperature;
    int expectedSpeed;
    String description;
  };

  TempTest tests[] = {
    { 20.0, 10, "Very Cool (20°C) - Minimal speed" },
    { 24.0, 10, "Cool (24°C) - Minimal speed" },
    { 25.0, 10, "At target baseline (25°C) - Minimal speed" },
    { 28.5, 25, "Slightly above target (28.5°C) - Low speed" },
    { 29.0, 40, "0.5°C above target (29°C) - Moderate speed" },
    { 29.5, 60, "1°C above target (29.5°C) - Higher speed" },
    { 30.5, 80, "2°C above target (30.5°C) - High speed" },
    { 32.0, 100, "4°C above target (32°C) - Full speed" },
    { 35.0, 100, "Very Hot (35°C) - Full speed" }
  };

  int testCount = sizeof(tests) / sizeof(TempTest);
  int passedTests = 0;

  for (int i = 0; i < testCount; i++) {
    Serial.printf("\n📋 Test %d/%d: %s\n", i + 1, testCount, tests[i].description.c_str());

    // Set test temperature
    testTemperature = tests[i].temperature;

    // Force sensor reading and control logic
    readAllSensors();
    controlFanSpeed();

    // Check result
    bool testPassed = (fanControl.currentFanSpeed == tests[i].expectedSpeed);
    if (testPassed) {
      Serial.printf("✅ PASS - Temp: %.1f°C → Fan: %d%% (Expected: %d%%)\n",
                    tests[i].temperature, fanControl.currentFanSpeed, tests[i].expectedSpeed);
      passedTests++;
    } else {
      Serial.printf("❌ FAIL - Temp: %.1f°C → Fan: %d%% (Expected: %d%%)\n",
                    tests[i].temperature, fanControl.currentFanSpeed, tests[i].expectedSpeed);
    }

    delay(1000);  // Brief pause between tests
  }

  // Summary
  Serial.printf("\n📊 TEST SUMMARY: %d/%d tests passed (%.1f%%)\n",
                passedTests, testCount, (float)passedTests / testCount * 100);

  if (passedTests == testCount) {
    Serial.println("🎉 ALL TEMPERATURE LOGIC TESTS PASSED!");
  } else {
    Serial.println("⚠️ Some temperature logic tests failed - check implementation");
  }

  // Restore original state
  temperatureTestMode = originalTestMode;
  fanControl.autoMode = originalAutoMode;
  testTemperature = originalTemp;

  Serial.println("═══════════════════════════════════════════\n");
}

// Comprehensive TRIAC Control Test
void testTRIACControlLogic() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║       TRIAC CONTROL LOGIC TEST        ║");
  Serial.println("╚════════════════════════════════════════╝");

  // Save current state
  bool originalAutoMode = fanControl.autoMode;
  int originalTargetSpeed = fanControl.targetFanSpeed;

  // Disable auto mode for manual control
  fanControl.autoMode = false;

  // Test different fan speeds
  int testSpeeds[] = { 0, 25, 50, 75, 100, 60, 30, 90, 10 };
  int testCount = sizeof(testSpeeds) / sizeof(int);
  int passedTests = 0;

  for (int i = 0; i < testCount; i++) {
    Serial.printf("\n🔧 Test %d/%d: Setting TRIAC to %d%%\n", i + 1, testCount, testSpeeds[i]);

    // Set target speed
    fanControl.targetFanSpeed = testSpeeds[i];

    // Force control logic
    controlFanSpeed();

    // Verify TRIAC response
    int triacPower = triac.getPower();
    bool testPassed = (fanControl.currentFanSpeed == testSpeeds[i] && triacPower == testSpeeds[i]);

    if (testPassed) {
      Serial.printf("✅ PASS - Target: %d%%, Current: %d%%, TRIAC: %d%%\n",
                    testSpeeds[i], fanControl.currentFanSpeed, triacPower);
      passedTests++;
    } else {
      Serial.printf("❌ FAIL - Target: %d%%, Current: %d%%, TRIAC: %d%%\n",
                    testSpeeds[i], fanControl.currentFanSpeed, triacPower);

      if (fanControl.currentFanSpeed != testSpeeds[i]) {
        Serial.println("   ↳ Issue: Current fan speed mismatch");
      }
      if (triacPower != testSpeeds[i]) {
        Serial.println("   ↳ Issue: TRIAC power reading mismatch");
      }
    }

    delay(2000);  // Allow time to observe actual fan behavior
  }

  // Test rapid speed changes
  Serial.println("\n🚀 Testing rapid speed changes...");
  int rapidSpeeds[] = { 100, 0, 50, 25, 75 };
  for (int speed : rapidSpeeds) {
    fanControl.targetFanSpeed = speed;
    controlFanSpeed();
    Serial.printf("Quick test: %d%% → Current: %d%%\n", speed, fanControl.currentFanSpeed);
    delay(500);
  }

  // Summary
  Serial.printf("\n📊 TRIAC TEST SUMMARY: %d/%d tests passed (%.1f%%)\n",
                passedTests, testCount, (float)passedTests / testCount * 100);

  if (passedTests == testCount) {
    Serial.println("🎉 ALL TRIAC CONTROL TESTS PASSED!");
  } else {
    Serial.println("⚠️ Some TRIAC control tests failed - check hardware/wiring");
  }

  // Restore original state
  fanControl.autoMode = originalAutoMode;
  fanControl.targetFanSpeed = originalTargetSpeed;

  Serial.println("═══════════════════════════════════════════\n");
}

// Quick test of key temperature points
void testQuickTemperaturePoints() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║        QUICK TEMPERATURE TEST         ║");
  Serial.println("╚════════════════════════════════════════╝");

  // Save current state
  bool originalTestMode = temperatureTestMode;
  bool originalAutoMode = fanControl.autoMode;

  // Enable test mode and auto mode
  temperatureTestMode = true;
  fanControl.autoMode = true;

  // Key test points
  float keyTemps[] = { 20.0, 25.0, 29.0, 30.0, 32.0 };
  String descriptions[] = { "Cool", "Target", "Warm", "Hot", "Very Hot" };

  for (int i = 0; i < 5; i++) {
    testTemperature = keyTemps[i];
    readAllSensors();
    controlFanSpeed();

    Serial.printf("🌡️ %s (%.1f°C) → Fan: %d%%\n",
                  descriptions[i].c_str(), keyTemps[i], fanControl.currentFanSpeed);
    delay(1000);
  }

  // Restore original state
  temperatureTestMode = originalTestMode;
  fanControl.autoMode = originalAutoMode;

  Serial.println("═══════════════════════════════════════════\n");
}
