#include "ESPCommunication.h"

ESPCommunication::ESPCommunication(int rx, int tx) {
    rxPin = rx;
    txPin = tx;
    initialized = false;
    serial = new SoftwareSerial(rxPin, txPin);
}

void ESPCommunication::begin(long baudRate) {
    serial->begin(baudRate);
    initialized = true;
    Serial.println("ESP8266 Communication initialized on D" + String(rxPin) + "(RX) and D" + String(txPin) + "(TX)");
}

void ESPCommunication::sendData(String data) {
    if (!initialized) return;
    
    // Add start and end markers for data integrity
    String message = "<" + data + ">";
    serial->println(message);
    Serial.println("Sent to ESP32: " + message);
}

void ESPCommunication::sendCommand(String command) {
    String data = "CMD:" + command;
    sendData(data);
}

void ESPCommunication::sendFirebaseStatus(String status) {
    String data = "FIREBASE:" + status;
    sendData(data);
}

void ESPCommunication::requestTemperature() {
    sendCommand("GET_TEMP");
}

void ESPCommunication::requestFanSpeed() {
    sendCommand("GET_FAN");
}

void ESPCommunication::setFanSpeed(int speed) {
    String data = "SET_FAN:" + String(speed);
    sendData(data);
}

String ESPCommunication::receiveData() {
    if (!initialized || !serial->available()) return "";
    
    String receivedData = "";
    while (serial->available()) {
        char c = serial->read();
        receivedData += c;
        delay(1); // Small delay to ensure complete data reception
    }
    
    // Remove start and end markers
    receivedData.trim();
    if (receivedData.startsWith("<") && receivedData.endsWith(">")) {
        receivedData = receivedData.substring(1, receivedData.length() - 1);
    }
    
    if (receivedData.length() > 0) {
        Serial.println("Received from ESP32: " + receivedData);
    }
    
    return receivedData;
}

bool ESPCommunication::isDataAvailable() {
    return initialized && serial->available();
}

void ESPCommunication::processIncomingData() {
    if (!isDataAvailable()) return;
    
    String data = receiveData();
    if (data.length() == 0) return;
    
    // Process different types of data from ESP32
    if (data.startsWith("TEMP:")) {
        String tempStr = data.substring(5);
        float temperature = tempStr.toFloat();
        Serial.println("Temperature received: " + String(temperature) + "°C");
        // This data can be sent to Firebase
        
    } else if (data.startsWith("FAN:")) {
        String fanStr = data.substring(4);
        int fanSpeed = fanStr.toInt();
        Serial.println("Fan speed received: " + String(fanSpeed) + "%");
        // This data can be sent to Firebase
        
    } else if (data.startsWith("SENSORS:")) {
        String sensorsStr = data.substring(8);
        Serial.println("Sensor data received: " + sensorsStr);
        // Parse and send to Firebase: temp,humidity,voltage,current
        
    } else if (data.startsWith("STATUS:")) {
        String status = data.substring(7);
        Serial.println("Status received: " + status);
    }
}