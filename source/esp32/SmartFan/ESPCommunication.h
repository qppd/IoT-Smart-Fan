#ifndef ESP_COMMUNICATION_H
#define ESP_COMMUNICATION_H

#include <Arduino.h>
#include <HardwareSerial.h>

struct ControlSettings {
    int targetFanSpeed = 50;
    float targetTemperature = 28.0;
    bool buzzerEnabled = true;
    String firebaseStatus = "DISCONNECTED";
    String wifiStatus = "DISCONNECTED";
    unsigned long lastUpdate = 0;
};

class ESPCommunication {
private:
    HardwareSerial* serial;
    int rxPin;
    int txPin;
    bool initialized;
    ControlSettings controlSettings;
    
    void sendMessage(String type, String data);
    String parseMessage(String message);
    
public:
    ESPCommunication(int rx, int tx);
    void begin(long baudRate = 9600);
    
    // Enhanced data sending methods
    void sendTemperature(float temperature);
    void sendHumidity(float humidity);
    void sendVoltage(float voltage);
    void sendCurrent(float current);
    void sendFanSpeed(int speed);
    void sendBuzzerStatus(bool active);
    void sendStatus(String status);
    void sendAllSensorData(float temp, float humidity, float voltage, float current, int fanSpeed);
    
    // Response methods
    void respondToPing();
    void sendSystemStatus();
    
    // Data reception methods
    String receiveData();
    bool isDataAvailable();
    void processIncomingData();
    ControlSettings getControlSettings();
    bool hasNewCommands();
    
    // Communication health check
    bool testCommunication();
    void printStatus();
};

#endif // ESP_COMMUNICATION_H