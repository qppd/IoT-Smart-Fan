
#include <Arduino.h>
#include "PinConfig.h"
#include "FirebaseConfig.h"
#include "firebase_credentials.h"
#include "WiFiManager.h"
#include "ESPCommunication.h"

FirebaseManager firebaseManager;
ESPCommunication espComm(ESP_SERIAL_RX, ESP_SERIAL_TX);

// Smart Fan data variables
struct SmartFanData {
    float temperature = 25.0;
    int fanSpeed = 128;
    float voltage = 220.0;
    float current = 0.5;
    float kwh = 0.01;
    String mode = "auto";
    String deviceId = "SmartFan_ESP8266_001";
};

SmartFanData fanData;
unsigned long lastDataSend = 0;
unsigned long lastNotification = 0;
const unsigned long DATA_SEND_INTERVAL = 5000;  // Send data every 5 seconds
const unsigned long NOTIFICATION_INTERVAL = 30000; // Send notifications every 30 seconds

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Smart Fan ESP8266 with Firebase ===");
    
    pinMode(WIFI_RESET_PIN, INPUT_PULLUP);
    
    // Initialize WiFi and Firebase
    setupWiFi();
    firebaseManager.begin();
    
    // ESP Communication setup
    espComm.begin(9600);
    
    Serial.println("Smart Fan ESP8266 Initialized Successfully!");
    
    // Send initial notification
    delay(2000);
    firebaseManager.sendMessage("Smart Fan", "System started successfully! 🌟");
    
    // Test communication with ESP32
    delay(3000);
    testESP8266Communication();
}

void loop() {
    // Process incoming data from ESP32
    espComm.processIncomingData();
    
    // Check for new sensor data from ESP32
    if (espComm.isDataAvailable()) {
        String receivedData = espComm.receiveData();
        if (receivedData.length() > 0) {
            parseReceivedData(receivedData);
        }
    }
    
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
    
    // Send status to ESP32 periodically
    static unsigned long lastStatusTime = 0;
    if (millis() - lastStatusTime > 10000) { // Send status every 10 seconds
        espComm.sendFirebaseStatus("CONNECTED");
        espComm.requestTemperature(); // Request fresh temperature data
        lastStatusTime = millis();
    }
    
    delay(100); // Small delay to prevent watchdog reset
}

void parseReceivedData(String data) {
    // Parse data received from ESP32
    // Expected format: "TEMP:25.5" or "SPEED:128" etc.
    if (data.startsWith("TEMP:")) {
        fanData.temperature = data.substring(5).toFloat();
        Serial.println("Updated temperature: " + String(fanData.temperature));
    }
    else if (data.startsWith("SPEED:")) {
        fanData.fanSpeed = data.substring(6).toInt();
        Serial.println("Updated fan speed: " + String(fanData.fanSpeed));
    }
    else if (data.startsWith("VOLTAGE:")) {
        fanData.voltage = data.substring(8).toFloat();
        Serial.println("Updated voltage: " + String(fanData.voltage));
    }
    else if (data.startsWith("CURRENT:")) {
        fanData.current = data.substring(8).toFloat();
        Serial.println("Updated current: " + String(fanData.current));
    }
    else if (data.startsWith("MODE:")) {
        fanData.mode = data.substring(5);
        Serial.println("Updated mode: " + fanData.mode);
    }
    else {
        Serial.println("ESP32 data: " + data);
    }
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
    int bottleSmall = map(fanData.temperature, 20, 40, 0, 50);
    int binLevel = map(fanData.current, 0, 2, 0, 100);
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
    
    Serial.println("📊 Data sent to Firebase");
}

void sendStatusNotification() {
    if (!firebaseManager.isReady()) return;
    
    String statusMessage = "Smart Fan Status:\n";
    statusMessage += "🌡️ Temp: " + String(fanData.temperature, 1) + "°C\n";
    statusMessage += "🌀 Speed: " + String(fanData.fanSpeed) + "\n";
    statusMessage += "⚡ Power: " + String(fanData.voltage * fanData.current, 1) + "W\n";
    statusMessage += "🔧 Mode: " + fanData.mode;
    
    // Send to all registered devices
    firebaseManager.sendMessageToAll("Smart Fan Update", statusMessage);
    
    Serial.println("📱 Status notification sent");
}

// Test function for ESP8266 communication
void testESP8266Communication() {
    Serial.println("=== ESP8266 Communication Test ===");
    
    // Send test messages
    espComm.sendData("TEST:ESP8266_HELLO");
    delay(500);
    espComm.sendCommand("GET_STATUS");
    delay(500);
    espComm.sendFirebaseStatus("CONNECTED");
    delay(500);
    espComm.setFanSpeed(50);
    delay(500);
    
    Serial.println("Test messages sent. Check ESP32 serial monitor for received data.");
    
    // Check for responses
    unsigned long testStart = millis();
    Serial.println("Waiting for ESP32 response...");
    while (millis() - testStart < 5000) { // Wait 5 seconds for response
        if (espComm.isDataAvailable()) {
            String response = espComm.receiveData();
            if (response.length() > 0) {
                Serial.println("ESP32 responded: " + response);
                break;
            }
        }
        delay(100);
    }
    
    Serial.println("=== ESP8266 Communication Test Complete ===");
}
