#ifndef ESP_COMMUNICATION_H
#define ESP_COMMUNICATION_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class ESPCommunication {
private:
    SoftwareSerial* serial;
    int rxPin;
    int txPin;
    bool initialized;
    
public:
    ESPCommunication(int rx, int tx);
    void begin(long baudRate = 9600);
    void sendData(String data);
    void sendCommand(String command);
    void sendFirebaseStatus(String status);
    void requestTemperature();
    void requestFanSpeed();
    void setFanSpeed(int speed);
    String receiveData();
    bool isDataAvailable();
    void processIncomingData();
};

#endif // ESP_COMMUNICATION_H