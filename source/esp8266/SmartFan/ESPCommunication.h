#ifndef ESP_COMMUNICATION_H
#define ESP_COMMUNICATION_H

#include <Arduino.h>
#include <SoftwareSerial.h>

struct SensorData {
    float temperature = 0.0;
    float humidity = 0.0;
    float voltage = 0.0;
    float current = 0.0;
    int fanSpeed = 0;
    bool buzzerActive = false;
    String status = "UNKNOWN";
    unsigned long lastUpdate = 0;
};

class ESPCommunication {
private:
    SoftwareSerial* serial;
    int rxPin;
    int txPin;
    bool initialized;
    SensorData lastSensorData;
    
    void sendMessage(String type, String data);
    String parseMessage(String message);
    
public:
    ESPCommunication(int rx, int tx);
    void begin(long baudRate = 9600);
    
    // Enhanced command methods
    void sendCommand(String command);
    void sendFirebaseStatus(String status);
    void sendWiFiStatus(String status);
    void setFanSpeed(int speed);
    void setTargetTemperature(float temp);
    void setMode(String mode);  // New method for mode control
    void requestAllSensors();
    void requestStatus();
    void triggerBuzzerAlert();
    
    // Data reception methods
    String receiveData();
    bool isDataAvailable();
    void processIncomingData();
    SensorData getLastSensorData();
    bool isDataFresh(unsigned long maxAge = 10000); // 10 seconds default
    
    // Communication health check
    bool testCommunication();
    void printStatus();
};

#endif // ESP_COMMUNICATION_H