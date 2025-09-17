#ifndef FIREBASE_CONFIG_H
#define FIREBASE_CONFIG_H

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include "firebase_credentials.h"

class FirebaseManager {
public:
    FirebaseManager();
    void begin();
    void updateDeviceCurrent(const String& deviceId, float temperature, int fanSpeed, const String& mode, unsigned long lastUpdate, float voltage, float current, float watt, float kwh);
    void logDeviceData(const String& deviceId, unsigned long timestamp, float temperature, int fanSpeed, float voltage, float current, float watt, float kwh);
    void resetWiFiSettings();
    bool isReady();
private:
    FirebaseData _fbdo;
};

#endif // FIREBASE_CONFIG_H
