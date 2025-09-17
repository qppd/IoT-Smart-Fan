
#include <Arduino.h>
#include "PinConfig.h"
#include "FirebaseConfig.h"
#include "firebase_credentials.h"
#include "WiFiManager.h"
#include "ESPCommunication.h"

FirebaseManager firebaseManager;
ESPCommunication espComm(ESP_SERIAL_RX, ESP_SERIAL_TX);

// Smart Fan data variables - now populated from ESP32
struct SmartFanData {
    float temperature = 25.0;
    float humidity = 50.0;
    int fanSpeed = 128;
    float voltage = 220.0;
    float current = 0.5;
    float kwh = 0.01;
    String mode = "auto";
    String deviceId = "SmartFan_ESP8266_001";
    bool esp32Connected = false;
};

SmartFanData fanData;
unsigned long lastDataSend = 0;
unsigned long lastNotification = 0;
unsigned long lastESP32Request = 0;
unsigned long lastConnCheck = 0;
const unsigned long DATA_SEND_INTERVAL = 5000;  // Send data every 5 seconds
const unsigned long NOTIFICATION_INTERVAL = 30000; // Send notifications every 30 seconds
const unsigned long ESP32_REQUEST_INTERVAL = 3000;  // Request ESP32 data every 3 seconds
const unsigned long CONN_CHECK_INTERVAL = 15000;   // Check ESP32 connection every 15 seconds

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Smart Fan ESP8266 - Firebase & WiFi Manager ===");
    
    pinMode(WIFI_RESET_PIN, INPUT_PULLUP);
    
    // Initialize WiFi and Firebase
    setupWiFi();
    firebaseManager.begin();
    
    // ESP Communication setup
    espComm.begin(9600);
    
    Serial.println("Smart Fan ESP8266 Initialized Successfully!");
    
    // Send initial notification
    delay(2000);
    firebaseManager.sendMessage("Smart Fan", "ESP8266 WiFi/Firebase module started! 🌟");
    
    // Test communication with ESP32
    delay(1000);
    testESP8266Communication();
}

void loop() {
    // Process incoming data from ESP32
    espComm.processIncomingData();
    
    // Update local data with ESP32 sensor readings
    updateDataFromESP32();
    
    // Send data to Firebase periodically
    if (millis() - lastDataSend > DATA_SEND_INTERVAL) {
        sendFirebaseData();
        lastDataSend = millis();
    }
    
    // Send periodic status notifications
    if (millis() - lastNotification > NOTIFICATION_INTERVAL) {
        sendStatusNotification();
        lastNotification = millis();
    }
    
    // Request fresh data from ESP32 periodically
    if (millis() - lastESP32Request > ESP32_REQUEST_INTERVAL) {
        requestESP32Data();
        lastESP32Request = millis();
    }
    
    // Check ESP32 connection health
    if (millis() - lastConnCheck > CONN_CHECK_INTERVAL) {
        checkESP32Connection();
        lastConnCheck = millis();
    }
    
    // Handle WiFi reset button
    handleWiFiReset();
    
    delay(100); // Small delay to prevent watchdog reset
}

void updateDataFromESP32() {
    SensorData sensorData = espComm.getLastSensorData();
    
    // Update fan data if ESP32 data is fresh (within 10 seconds)
    if (espComm.isDataFresh(10000)) {
        fanData.temperature = sensorData.temperature;
        fanData.humidity = sensorData.humidity;
        fanData.voltage = sensorData.voltage;
        fanData.current = sensorData.current;
        fanData.fanSpeed = sensorData.fanSpeed;
        fanData.esp32Connected = true;
        
        // Calculate kWh
        static unsigned long lastKwhUpdate = 0;
        if (lastKwhUpdate > 0) {
            float hours = (millis() - lastKwhUpdate) / 3600000.0;
            float watt = fanData.voltage * fanData.current;
            fanData.kwh += (watt * hours) / 1000.0;
        }
        lastKwhUpdate = millis();
    } else {
        fanData.esp32Connected = false;
    }
}

void requestESP32Data() {
    // Send status updates to ESP32
    String firebaseStatus = firebaseManager.isReady() ? "CONNECTED" : "DISCONNECTED";
    espComm.sendFirebaseStatus(firebaseStatus);
    
    // Send WiFi status
    String wifiStatus = WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED";
    espComm.sendWiFiStatus(wifiStatus);
    
    // Request fresh sensor data
    espComm.requestAllSensors();
}

void checkESP32Connection() {
    if (!fanData.esp32Connected) {
        Serial.println("⚠️ ESP32 connection lost - sending alert");
        if (firebaseManager.isReady()) {
            firebaseManager.sendMessage("Smart Fan Alert", "ESP32 sensor module disconnected! ⚠️");
        }
    }
    
    // Print communication status
    espComm.printStatus();
}

void sendFirebaseData() {
    if (!firebaseManager.isReady()) {
        Serial.println("Firebase not ready, skipping data send");
        return;
    }
    
    unsigned long now = millis() / 1000 + 1692620000; // Convert to timestamp
    float watt = fanData.voltage * fanData.current;
    
    // Update current device state
    firebaseManager.updateDeviceCurrent(
        fanData.deviceId, 
        fanData.temperature, 
        fanData.fanSpeed, 
        fanData.mode, 
        now, 
        fanData.voltage, 
        fanData.current, 
        watt, 
        fanData.kwh
    );
    
    // Log device data
    firebaseManager.logDeviceData(
        fanData.deviceId, 
        now, 
        fanData.temperature, 
        fanData.fanSpeed, 
        fanData.voltage, 
        fanData.current, 
        watt, 
        fanData.kwh
    );
    
    // Send PlasTech format data (for compatibility)
    int bottleLarge = map(fanData.fanSpeed, 0, 255, 0, 100);
    int bottleSmall = map(int(fanData.temperature), 20, 40, 0, 50);
    int binLevel = map(int(fanData.current * 100), 0, 200, 0, 100);
    int totalRewards = int(fanData.kwh * 10);
    int totalWeight = int(watt);
    int coinStock = 100;
    
    firebaseManager.sendSmartFanData(
        bottleLarge, 
        bottleSmall, 
        binLevel, 
        totalRewards, 
        totalWeight, 
        coinStock
    );
    
    String connStatus = fanData.esp32Connected ? "🟢" : "🔴";
    Serial.println("📊 Data sent to Firebase " + connStatus);
}

void sendStatusNotification() {
    if (!firebaseManager.isReady()) return;
    
    String connStatus = fanData.esp32Connected ? "🟢 Connected" : "🔴 Disconnected";
    String statusMessage = "Smart Fan Status:\n";
    statusMessage += "🌡️ Temp: " + String(fanData.temperature, 1) + "°C\n";
    statusMessage += "💧 Humidity: " + String(fanData.humidity, 1) + "%\n";
    statusMessage += "🌀 Speed: " + String(fanData.fanSpeed) + "\n";
    statusMessage += "⚡ Power: " + String(fanData.voltage * fanData.current, 1) + "W\n";
    statusMessage += "🔧 Mode: " + fanData.mode + "\n";
    statusMessage += "📡 ESP32: " + connStatus;
    
    // Send to all registered devices
    firebaseManager.sendMessageToAll("Smart Fan Update", statusMessage);
    
    Serial.println("📱 Status notification sent");
}

void handleWiFiReset() {
    if (digitalRead(WIFI_RESET_PIN) == LOW) {
        static unsigned long resetStartTime = 0;
        if (resetStartTime == 0) {
            resetStartTime = millis();
        }
        
        // If button held for 3 seconds, reset WiFi
        if (millis() - resetStartTime > 3000) {
            Serial.println("WiFi Reset requested!");
            resetWiFi();
            resetStartTime = 0;
        }
    } else {
        // Button released
        static unsigned long resetStartTime = 0;
        resetStartTime = 0;
    }
}

// Test function for ESP8266 communication
void testESP8266Communication() {
    Serial.println("=== ESP8266 Communication Test ===");
    
    bool testResult = espComm.testCommunication();
    
    if (testResult) {
        Serial.println("✅ ESP32 communication working!");
        if (firebaseManager.isReady()) {
            firebaseManager.sendMessage("Smart Fan", "ESP8266-ESP32 communication established! ✅");
        }
    } else {
        Serial.println("❌ ESP32 communication failed!");
        if (firebaseManager.isReady()) {
            firebaseManager.sendMessage("Smart Fan Alert", "ESP32 communication failed! ❌");
        }
    }
    
    Serial.println("=== ESP8266 Communication Test Complete ===");
}


