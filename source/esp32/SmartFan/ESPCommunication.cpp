#include "ESPCommunication.h"

ESPCommunication::ESPCommunication(int rx, int tx) {
    rxPin = rx;
    txPin = tx;
    initialized = false;
    serial = &Serial2; // Use Hardware Serial 2 on ESP32
}

void ESPCommunication::begin(long baudRate) {
    serial->begin(baudRate, SERIAL_8N1, rxPin, txPin);
    initialized = true;
    Serial.println("ESP32 Communication initialized on GPIO" + String(rxPin) + "(RX) and GPIO" + String(txPin) + "(TX)");
}

void ESPCommunication::sendData(String data) {
    if (!initialized) return;
    
    // Add start and end markers for data integrity
    String message = "<" + data + ">";
    serial->println(message);
    Serial.println("Sent to ESP8266: " + message);
}

void ESPCommunication::sendTemperature(float temperature) {
    String data = "TEMP:" + String(temperature, 2);
    sendData(data);
}

void ESPCommunication::sendFanSpeed(int speed) {
    String data = "FAN:" + String(speed);
    sendData(data);
}

void ESPCommunication::sendSensorData(float temp, float humidity, float voltage, float current) {
    String data = "SENSORS:" + String(temp, 2) + "," + String(humidity, 2) + "," + String(voltage, 2) + "," + String(current, 3);
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
        Serial.println("Received from ESP8266: " + receivedData);
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
    
    // Process different types of commands from ESP8266
    if (data.startsWith("CMD:")) {
        String command = data.substring(4);
        Serial.println("Command received: " + command);
        
        if (command == "GET_STATUS") {
            sendData("STATUS:OK");
        } else if (command == "GET_TEMP") {
            // This would be handled in the main loop to send actual temperature
            Serial.println("Temperature request received");
        }
    } else if (data.startsWith("SET_FAN:")) {
        String speedStr = data.substring(8);
        int newSpeed = speedStr.toInt();
        Serial.println("Fan speed command received: " + String(newSpeed));
        // This would be handled in the main loop to actually set fan speed
    }
}