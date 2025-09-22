#include "ESPCommunication.h"

ESPCommunication::ESPCommunication(int rx, int tx) {
    rxPin = rx;
    txPin = tx;
    initialized = false;
    serial = new SoftwareSerial(rxPin, txPin);
    lastSensorData.lastUpdate = 0;
}

void ESPCommunication::begin(long baudRate) {
    serial->begin(baudRate);
    initialized = true;
    Serial.println("ESP8266 Communication initialized on D" + String(rxPin) + "(RX) and D" + String(txPin) + "(TX)");
    
    // Send initialization message
    delay(100);
    sendMessage("INIT", "ESP8266_READY");
}

void ESPCommunication::sendMessage(String type, String data) {
    if (!initialized) return;
    
    String message = "<" + type + ":" + data + ">";
    serial->println(message);
    Serial.println("Sent to ESP32: " + message);
}

void ESPCommunication::sendCommand(String command) {
    sendMessage("CMD", command);
}

void ESPCommunication::sendFirebaseStatus(String status) {
    sendMessage("FIREBASE", status);
}

void ESPCommunication::sendWiFiStatus(String status) {
    sendMessage("WIFI", status);
}

void ESPCommunication::setFanSpeed(int speed) {
    speed = constrain(speed, 0, 100);
    sendMessage("SET_FAN", String(speed));
}

void ESPCommunication::setTargetTemperature(float temp) {
    sendMessage("SET_TEMP", String(temp, 1));
}

void ESPCommunication::setMode(String mode) {
    sendMessage("SET_MODE", mode);
}

void ESPCommunication::requestAllSensors() {
    sendCommand("GET_SENSORS");
}

void ESPCommunication::requestStatus() {
    sendCommand("GET_STATUS");
}

void ESPCommunication::triggerBuzzerAlert() {
    sendMessage("BUZZ", "ALERT");
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
        Serial.println("Received from ESP32: <" + parsedData + ">");
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
    
    // Parse different types of data from ESP32
    int colonPos = data.indexOf(':');
    if (colonPos == -1) return;
    
    String messageType = data.substring(0, colonPos);
    String messageData = data.substring(colonPos + 1);
    
    if (messageType == "TEMP") {
        lastSensorData.temperature = messageData.toFloat();
        lastSensorData.lastUpdate = millis();
        Serial.println("Temperature updated: " + String(lastSensorData.temperature, 1) + "°C");
        
    } else if (messageType == "HUMID") {
        lastSensorData.humidity = messageData.toFloat();
        lastSensorData.lastUpdate = millis();
        Serial.println("Humidity updated: " + String(lastSensorData.humidity, 1) + "%");
        
    } else if (messageType == "VOLT") {
        lastSensorData.voltage = messageData.toFloat();
        lastSensorData.lastUpdate = millis();
        Serial.println("Voltage updated: " + String(lastSensorData.voltage, 1) + "V");
        
    } else if (messageType == "CURR") {
        lastSensorData.current = messageData.toFloat();
        lastSensorData.lastUpdate = millis();
        Serial.println("Current updated: " + String(lastSensorData.current, 3) + "A");
        
    } else if (messageType == "FAN") {
        lastSensorData.fanSpeed = messageData.toInt();
        lastSensorData.lastUpdate = millis();
        Serial.println("Fan speed updated: " + String(lastSensorData.fanSpeed) + "%");
        
    } else if (messageType == "BUZZ") {
        lastSensorData.buzzerActive = (messageData == "ON");
        lastSensorData.lastUpdate = millis();
        Serial.println("Buzzer status: " + messageData);
        
    } else if (messageType == "STATUS") {
        lastSensorData.status = messageData;
        lastSensorData.lastUpdate = millis();
        Serial.println("ESP32 status: " + messageData);
        
    } else if (messageType == "ALL") {
        // Parse combined sensor data: temp,humid,volt,curr,fan
        int pos = 0;
        int nextPos = 0;
        
        // Temperature
        nextPos = messageData.indexOf(',', pos);
        if (nextPos > pos) {
            lastSensorData.temperature = messageData.substring(pos, nextPos).toFloat();
            pos = nextPos + 1;
        }
        
        // Humidity
        nextPos = messageData.indexOf(',', pos);
        if (nextPos > pos) {
            lastSensorData.humidity = messageData.substring(pos, nextPos).toFloat();
            pos = nextPos + 1;
        }
        
        // Voltage
        nextPos = messageData.indexOf(',', pos);
        if (nextPos > pos) {
            lastSensorData.voltage = messageData.substring(pos, nextPos).toFloat();
            pos = nextPos + 1;
        }
        
        // Current
        nextPos = messageData.indexOf(',', pos);
        if (nextPos > pos) {
            lastSensorData.current = messageData.substring(pos, nextPos).toFloat();
            pos = nextPos + 1;
        }
        
        // Fan speed
        if (pos < messageData.length()) {
            lastSensorData.fanSpeed = messageData.substring(pos).toInt();
        }
        
        lastSensorData.lastUpdate = millis();
        Serial.println("All sensor data updated from ESP32");
        
    } else {
        Serial.println("Unknown message type from ESP32: " + messageType + " = " + messageData);
    }
}

SensorData ESPCommunication::getLastSensorData() {
    return lastSensorData;
}

bool ESPCommunication::isDataFresh(unsigned long maxAge) {
    return (millis() - lastSensorData.lastUpdate) < maxAge;
}

bool ESPCommunication::testCommunication() {
    Serial.println("Testing ESP8266 <-> ESP32 communication...");
    
    // Send test message
    sendMessage("TEST", "ESP8266_PING");
    
    // Wait for response
    unsigned long startTime = millis();
    while (millis() - startTime < 3000) { // 3 second timeout
        if (isDataAvailable()) {
            String response = receiveData();
            if (response.indexOf("ESP32_PONG") >= 0) {
                Serial.println("Communication test PASSED!");
                return true;
            }
        }
        delay(100);
    }
    
    Serial.println("Communication test FAILED - no response from ESP32");
    return false;
}

void ESPCommunication::printStatus() {
    Serial.println("=== ESP Communication Status ===");
    Serial.println("Initialized: " + String(initialized ? "YES" : "NO"));
    Serial.println("RX Pin: D" + String(rxPin) + ", TX Pin: D" + String(txPin));
    Serial.println("Last sensor update: " + String((millis() - lastSensorData.lastUpdate) / 1000) + " seconds ago");
    Serial.println("Temperature: " + String(lastSensorData.temperature, 1) + "°C");
    Serial.println("Humidity: " + String(lastSensorData.humidity, 1) + "%");
    Serial.println("Voltage: " + String(lastSensorData.voltage, 1) + "V");
    Serial.println("Current: " + String(lastSensorData.current, 3) + "A");
    Serial.println("Fan Speed: " + String(lastSensorData.fanSpeed) + "%");
    Serial.println("ESP32 Status: " + lastSensorData.status);
    Serial.println("==============================");
}