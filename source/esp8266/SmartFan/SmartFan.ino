
#include <Arduino.h>
#include "PinConfig.h"
#include "FirebaseConfig.h"
#include "firebase_credentials.h"
#include "WiFiManager.h"
#include "ESPCommunication.h"
#include "NTPConfig.h"

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
const unsigned long DATA_SEND_INTERVAL = 60000;  // Send data every 5 seconds
const unsigned long NOTIFICATION_INTERVAL = 120000; // Send notifications every 2 hours
const unsigned long ESP32_REQUEST_INTERVAL = 30000;  // Request ESP32 data every 3 seconds
const unsigned long CONN_CHECK_INTERVAL = 150000;   // Check ESP32 connection every 15 seconds

// LED indicator control functions
void setLEDState(bool state) {
    digitalWrite(LED_PIN, state ? HIGH : LOW);
}

void blinkLED(int times, int delayMs = 200) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(delayMs);
        digitalWrite(LED_PIN, LOW);
        if (i < times - 1) delay(delayMs);
    }
}

void updateLEDStatus() {
    static unsigned long lastLEDUpdate = 0;
    static bool ledState = false;
    
    if (millis() - lastLEDUpdate > 1000) {  // Update LED status every second
        if (!WiFi.isConnected()) {
            // WiFi disconnected - fast blink
            ledState = !ledState;
            setLEDState(ledState);
        } else if (!firebaseManager.isReady()) {
            // WiFi connected but Firebase not ready - slow blink
            static int blinkCounter = 0;
            if (blinkCounter % 2 == 0) {
                ledState = !ledState;
                setLEDState(ledState);
            }
            blinkCounter = (blinkCounter + 1) % 4;
        } else if (!fanData.esp32Connected) {
            // Firebase ready but ESP32 disconnected - double blink pattern
            static int patternStep = 0;
            switch (patternStep) {
                case 0: case 2: setLEDState(HIGH); break;
                case 1: case 3: setLEDState(LOW); break;
                default: setLEDState(LOW); break;
            }
            patternStep = (patternStep + 1) % 6;
        } else {
            // All systems operational - solid on
            setLEDState(true);
        }
        lastLEDUpdate = millis();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Smart Fan ESP8266 - Firebase & WiFi Manager ===");
    
    // Print initial memory status
    Serial.printf("Initial Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Initial Heap Fragmentation: %d%%\n", ESP.getHeapFragmentation());
    
    // Initialize pins
    pinMode(WIFI_RESET_PIN, INPUT_PULLUP);  // Disabled - pin D8 stuck LOW
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // Start with LED off
    
    // Indicate startup with LED blink
    blinkLED(3, 100);
    
    // Initialize WiFi and Firebase
    setupWiFi();
    
    // Check memory after WiFi setup
    Serial.printf("After WiFi - Free Heap: %d bytes\n", ESP.getFreeHeap());
    
    // WiFi connected indicator
    blinkLED(2, 150);
    
    firebaseManager.begin();
    
    // Check memory after Firebase setup
    Serial.printf("After Firebase - Free Heap: %d bytes\n", ESP.getFreeHeap());

    //firebaseManager.resetWiFiSettings();
    
    // ESP Communication setup
    espComm.begin(9600);
    
    // Set ESP communication reference in Firebase manager for real-time control
    firebaseManager.setESPCommunication(&espComm);
    
    Serial.println(getCurrentLogPrefix() + "Smart Fan ESP8266 Initialized Successfully!");
    Serial.printf("Final Free Heap: %d bytes\n", ESP.getFreeHeap());
    
    // Initialization complete indicator
    blinkLED(5, 100);
    
    // Send initial notification - TEMPORARILY DISABLED
    delay(2000);
    // firebaseManager.sendMessage("Smart Fan", "ESP8266 WiFi/Firebase module started! 🌟");
    Serial.println(getCurrentLogPrefix() + "📲 FCM disabled - system started successfully");
    
    // Test communication with ESP32
    delay(1000);
    testESP8266Communication();
}

void loop() {
    // Process incoming data from ESP32
    espComm.processIncomingData();
    
    // Handle Firebase streams for real-time control
    if (firebaseManager.isReady()) {
        firebaseManager.handleStreams();
    }
    
    // Update local data with ESP32 sensor readings
    updateDataFromESP32();
    
    // Send data to Firebase periodically
    if (millis() - lastDataSend > DATA_SEND_INTERVAL) {
        // Check memory before Firebase operations
        if (ESP.getFreeHeap() < 8000) {  // Less than 8KB free
            Serial.printf("⚠️ Low memory warning: %d bytes free\n", ESP.getFreeHeap());
        }
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
    
    // Handle WiFi reset button - DISABLED (Pin D8 stuck LOW)
    handleWiFiReset();
    
    // Update LED status indicator
    updateLEDStatus();
    
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
        Serial.println(getCurrentLogPrefix() + "⚠️ ESP32 connection lost - sending alert");
        // FCM temporarily disabled
        // if (firebaseManager.isReady()) {
        //     firebaseManager.sendMessage("Smart Fan Alert", "ESP32 sensor module disconnected! ⚠️");
        // }
    }
    
    // Print communication status
    espComm.printStatus();
}

void sendFirebaseData() {
    if (!firebaseManager.isReady()) {
        Serial.println(getCurrentLogPrefix() + "Firebase not ready, skipping data send");
        return;
    }
    
    unsigned long now = getNTPTimestampWithFallback(); // Use NTP timestamp with fallback
    float watt = fanData.voltage * fanData.current;
    
    // Log timestamp source for debugging
    static unsigned long lastTimestampLog = 0;
    if (millis() - lastTimestampLog > 300000) { // Log every 5 minutes
        String timeSource = isNTPSynced() ? "NTP" : "Fallback";
        Serial.println(getCurrentLogPrefix() + "🕐 Using " + timeSource + " timestamp: " + String(now));
        lastTimestampLog = millis();
    }
    
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
    Serial.println(getCurrentLogPrefix() + "📊 Data sent to Firebase " + connStatus);
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
    
    // Send to all registered devices - FCM temporarily disabled
    // firebaseManager.sendMessageToAll("Smart Fan Update", statusMessage);
    Serial.println(getCurrentLogPrefix() + "📱 Status notification prepared (FCM disabled): " + statusMessage);
}

void handleWiFiReset() {
    // FUNCTION DISABLED: Pin D8 (WIFI_RESET_PIN) is stuck LOW causing constant resets
    // Hardware issue detected - pin appears to be shorted to ground
    // To re-enable: fix hardware issue or change to different pin in PinConfig.h
   
    
    static unsigned long resetStartTime = 0;
    static bool resetIndicatorShown = false;
    static unsigned long lastDebugPrint = 0;
    
    // Debug: Print pin state every 5 seconds
    if (millis() - lastDebugPrint > 5000) {
        int pinState = digitalRead(WIFI_RESET_PIN);
        Serial.printf("DEBUG: WIFI_RESET_PIN (D8) state: %s\n", pinState == LOW ? "LOW" : "HIGH");
        lastDebugPrint = millis();
    }
    
    if (digitalRead(WIFI_RESET_PIN) == LOW) {
        if (resetStartTime == 0) {
            resetStartTime = millis();
            resetIndicatorShown = false;
            Serial.println("DEBUG: WiFi reset pin detected LOW - starting timer");
        }
        
        // Show visual feedback during reset process
        if (!resetIndicatorShown && (millis() - resetStartTime > 1000)) {
            // Rapid blink to indicate reset button detected
            blinkLED(10, 50);
            resetIndicatorShown = true;
            Serial.println("DEBUG: Reset button held for 1 second");
        }
        
        // If button held for 3 seconds, reset WiFi
        if (millis() - resetStartTime > 3000) {
            Serial.println("WiFi Reset requested!");
            Serial.println("DEBUG: Pin has been LOW for 3+ seconds - triggering reset");
            
            // Show reset confirmation with long blinks
            for (int i = 0; i < 3; i++) {
                digitalWrite(LED_PIN, HIGH);
                delay(500);
                digitalWrite(LED_PIN, LOW);
                delay(500);
            }
            
            firebaseManager.resetWiFiSettings();
            resetStartTime = 0;
        }
    } else {
        // Button released
        if (resetStartTime != 0) {
            Serial.println("DEBUG: WiFi reset pin released");
        }
        resetStartTime = 0;
        resetIndicatorShown = false;
    }
}

// Test function for ESP8266 communication
void testESP8266Communication() {
    Serial.println(getCurrentLogPrefix() + "=== ESP8266 Communication Test ===");
    
    bool testResult = espComm.testCommunication();
    
    if (testResult) {
        Serial.println(getCurrentLogPrefix() + "✅ ESP32 communication working!");
        // FCM temporarily disabled
        // if (firebaseManager.isReady()) {
        //     firebaseManager.sendMessage("Smart Fan", "ESP8266-ESP32 communication established! ✅");
        // }
    } else {
        Serial.println(getCurrentLogPrefix() + "❌ ESP32 communication failed!");
        // FCM temporarily disabled
        // if (firebaseManager.isReady()) {
        //     firebaseManager.sendMessage("Smart Fan Alert", "ESP32 communication failed! ❌");
        // }
    }
    
    Serial.println(getCurrentLogPrefix() + "=== ESP8266 Communication Test Complete ===");
}


