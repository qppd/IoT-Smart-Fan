#ifndef ESP_COMMUNICATION_H
#define ESP_COMMUNICATION_H

#include <Arduino.h>
#include <HardwareSerial.h>

class ESPCommunication {
private:
    HardwareSerial* serial;
    int rxPin;
    int txPin;
    bool initialized;
    
public:
    ESPCommunication(int rx, int tx);
    void begin(long baudRate = 9600);
    void sendData(String data);
    void sendTemperature(float temperature);
    void sendFanSpeed(int speed);
    void sendSensorData(float temp, float humidity, float voltage, float current);
    String receiveData();
    bool isDataAvailable();
    void processIncomingData();
};

#endif // ESP_COMMUNICATION_H