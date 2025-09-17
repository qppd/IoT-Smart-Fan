#include "ESPCommunication.h"

ESPCommunication::ESPCommunication(int rx, int tx) {
    rxPin = rx;
    txPin = tx;
    initialized = false;
    serial = &Serial2; // Use Hardware Serial 2 on ESP32
    controlSettings.lastUpdate = 0;
}

void ESPCommunication::begin(long baudRate) {
    serial->begin(baudRate, SERIAL_8N1, rxPin, txPin);
    initialized = true;
    Serial.println("ESP32 Communication initialized on GPIO" + String(rxPin) + "(RX) and GPIO" + String(txPin) + "(TX)");
    
    // Send initialization message
    delay(100);
    sendMessage("INIT", "ESP32_READY");
}

void ESPCommunication::sendMessage(String type, String data) {
    if (!initialized) return;
    
    String message = "<" + type + ":" + data + ">";
    serial->println(message);
    Serial.println("Sent to ESP8266: " + message);
}

void ESPCommunication::sendTemperature(float temperature) {
    sendMessage("TEMP", String(temperature, 2));
}

void ESPCommunication::sendHumidity(float humidity) {
    sendMessage("HUMID", String(humidity, 2));
}

void ESPCommunication::sendVoltage(float voltage) {
    sendMessage("VOLT", String(voltage, 2));
}

void ESPCommunication::sendCurrent(float current) {
    sendMessage("CURR", String(current, 3));
}

void ESPCommunication::sendFanSpeed(int speed) {
    sendMessage("FAN", String(speed));
}

void ESPCommunication::sendBuzzerStatus(bool active) {
    sendMessage("BUZZ", active ? "ON" : "OFF");
}

void ESPCommunication::sendStatus(String status) {
    sendMessage("STATUS", status);
}

void ESPCommunication::sendAllSensorData(float temp, float humidity, float voltage, float current, int fanSpeed) {
    String data = String(temp, 2) + "," + String(humidity, 2) + "," + 
                  String(voltage, 2) + "," + String(current, 3) + "," + String(fanSpeed);
    sendMessage("ALL", data);
}

void ESPCommunication::respondToPing() {
    sendMessage("TEST", "ESP32_PONG");
}

void ESPCommunication::sendSystemStatus() {
    sendMessage("STATUS", "RUNNING");
}

String ESPCommunication::parseMessage(String message) {
    // Remove start and end markers
    message.trim();
    if (message.startsWith("<") && message.endsWith(">")) {
        return message.substring(1, message.length() - 1);
    }
    return message;
}

String ESPCommunication::receiveData() {
    if (!initialized || !serial->available()) return "";
    
    String receivedData = "";
    unsigned long startTime = millis();
    
    // Read until we get a complete message or timeout
    while (millis() - startTime < 1000) { // 1 second timeout
        if (serial->available()) {
            char c = serial->read();
            receivedData += c;
            
            // Check if we have a complete message
            if (receivedData.endsWith(">") && receivedData.indexOf("<") >= 0) {
                break;
            }
        }
        delay(1);
    }
    
    String parsedData = parseMessage(receivedData);
    
    if (parsedData.length() > 0) {
        Serial.println("Received from ESP8266: <" + parsedData + ">");
    }
    
    return parsedData;
}

bool ESPCommunication::isDataAvailable() {
    return initialized && serial->available();
}

void ESPCommunication::processIncomingData() {
    if (!isDataAvailable()) return;
    
    String data = receiveData();
    if (data.length() == 0) return;
    
    // Parse different types of commands from ESP8266
    int colonPos = data.indexOf(':');
    if (colonPos == -1) return;
    
    String messageType = data.substring(0, colonPos);
    String messageData = data.substring(colonPos + 1);
    
    if (messageType == "CMD") {
        Serial.println("Command received: " + messageData);
        
        if (messageData == "GET_STATUS") {
            sendSystemStatus();
        } else if (messageData == "GET_SENSORS") {
            // This will be handled in main loop to send actual sensor readings
            Serial.println("Sensor data request received - will send in main loop");
        }
        
    } else if (messageType == "SET_FAN") {
        int newSpeed = messageData.toInt();
        newSpeed = constrain(newSpeed, 0, 100);
        controlSettings.targetFanSpeed = newSpeed;
        controlSettings.lastUpdate = millis();
        Serial.println("Fan speed command: " + String(newSpeed) + "%");
        
    } else if (messageType == "SET_TEMP") {
        float newTemp = messageData.toFloat();
        controlSettings.targetTemperature = newTemp;
        controlSettings.lastUpdate = millis();
        Serial.println("Target temperature set: " + String(newTemp, 1) + "°C");
        
    } else if (messageType == "FIREBASE") {
        controlSettings.firebaseStatus = messageData;
        controlSettings.lastUpdate = millis();
        Serial.println("Firebase status: " + messageData);
        
    } else if (messageType == "WIFI") {
        controlSettings.wifiStatus = messageData;
        controlSettings.lastUpdate = millis();
        Serial.println("WiFi status: " + messageData);
        
    } else if (messageType == "BUZZ") {
        if (messageData == "ALERT") {
            // This will be handled in main loop to trigger buzzer
            Serial.println("Buzzer alert command received");
        }
        
    } else if (messageType == "TEST") {
        if (messageData == "ESP8266_PING") {
            respondToPing();
        }
        
    } else if (messageType == "INIT") {
        Serial.println("ESP8266 initialization: " + messageData);
        
    } else {
        Serial.println("Unknown command from ESP8266: " + messageType + " = " + messageData);
    }
}

ControlSettings ESPCommunication::getControlSettings() {
    return controlSettings;
}

bool ESPCommunication::hasNewCommands() {
    return (millis() - controlSettings.lastUpdate) < 1000; // Commands fresh within 1 second
}

bool ESPCommunication::testCommunication() {
    Serial.println("Testing ESP32 <-> ESP8266 communication...");
    
    // Send test message
    sendMessage("TEST", "ESP32_PING");
    
    // Wait for response
    unsigned long startTime = millis();
    while (millis() - startTime < 3000) { // 3 second timeout
        if (isDataAvailable()) {
            String response = receiveData();
            if (response.indexOf("ESP8266_PONG") >= 0) {
                Serial.println("Communication test PASSED!");
                return true;
            }
        }
        delay(100);
    }
    
    Serial.println("Communication test FAILED - no response from ESP8266");
    return false;
}

void ESPCommunication::printStatus() {
    Serial.println("=== ESP Communication Status ===");
    Serial.println("Initialized: " + String(initialized ? "YES" : "NO"));
    Serial.println("RX Pin: GPIO" + String(rxPin) + ", TX Pin: GPIO" + String(txPin));
    Serial.println("Last command: " + String((millis() - controlSettings.lastUpdate) / 1000) + " seconds ago");
    Serial.println("Target Fan Speed: " + String(controlSettings.targetFanSpeed) + "%");
    Serial.println("Target Temperature: " + String(controlSettings.targetTemperature, 1) + "°C");
    Serial.println("Firebase Status: " + controlSettings.firebaseStatus);
    Serial.println("WiFi Status: " + controlSettings.wifiStatus);
    Serial.println("==============================");
}