
#ifndef FIREBASECONFIG_H
#define FIREBASECONFIG_H

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "firebase_credentials.h"

#define MAX_TOKENS 10

class FirebaseManager {
public:
    FirebaseManager();
    void begin();
    void initWiFi();
    
    // Device data management
    void updateDeviceCurrent(const String& deviceId, float temperature, int fanSpeed, const String& mode, unsigned long lastUpdate, float voltage, float current, float watt, float kwh);
    void logDeviceData(const String& deviceId, unsigned long timestamp, float temperature, int fanSpeed, float voltage, float current, float watt, float kwh);
    
    // FCM messaging
    void sendMessageToAll(const String& title, const String& body);
    void beginTokenStream();
    
    // Status check
    bool isReady();

private:
    FirebaseData _fbdo;
    FirebaseAuth _auth;
    FirebaseConfig _config;
    
    // Token management
    String _deviceTokens[MAX_TOKENS];
    int _tokenCount;
    FirebaseData _tokenStream;
    String _tokenParentPath;
    
    // Static instance for callbacks
    static FirebaseManager* _instance;
    
    // Private helper methods
    static void tokenStreamCallback(MultiPathStream stream);
    static void tokenStreamTimeoutCallback(bool timeout);
};

#endif // FIREBASECONFIG_H
