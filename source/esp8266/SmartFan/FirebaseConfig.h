#ifndef FIREBASE_CONFIG_H
#define FIREBASE_CONFIG_H

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "firebase_credentials.h"

#define MAX_TOKENS 10  // Maximum number of device tokens to store

class FirebaseManager {
public:
    FirebaseManager();
    void begin();
    void updateDeviceCurrent(const String& deviceId, float temperature, int fanSpeed, const String& mode, unsigned long lastUpdate, float voltage, float current, float watt, float kwh);
    void logDeviceData(const String& deviceId, unsigned long timestamp, float temperature, int fanSpeed, float voltage, float current, float watt, float kwh);
    void sendSmartFanData(float temperature, int fanSpeed, float voltage, float current, float watt, float kwh);
    void sendMessage(String title, String body);
    void sendMessageToAll(String title, String body);
    void resetWiFiSettings();
    bool isReady();
    
    // Public Firebase objects for callbacks
    FirebaseData fbdo;
    FirebaseData tokenStream;
    
private:
    FirebaseAuth auth;
    FirebaseConfig config;
    
    // Device token management
    String deviceTokens[MAX_TOKENS];
    int tokenCount;
    
    // Stream configuration
    String tokenParentPath;
    String tokenPaths[1];
    
    // Internal methods
    void initializeTokenStream();
    static void tokenStreamCallback(MultiPathStream stream);
    static void tokenStreamTimeoutCallback(bool timeout);
};

// Global instance for callback access
extern FirebaseManager* globalFirebaseManager;

#endif // FIREBASE_CONFIG_H
