#ifndef FIREBASE_CONFIG_H
#define FIREBASE_CONFIG_H

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "firebase_credentials.h"
#include "NTPConfig.h"

#define MAX_TOKENS 10  // Maximum number of device tokens to store

// Forward declaration for communication interface
class ESPCommunication;

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
    
    // Stream management
    void initializeStreams();
    void handleStreams();
    void setESPCommunication(ESPCommunication* comm);
    
    // Manual check for tokens (non-realtime)
    void checkTokensUpdate();
    
    // Public Firebase objects for callbacks
    FirebaseData fbdo;
    FirebaseData controlStream;  // For manual/auto and fan speed
    
private:
    FirebaseAuth auth;
    FirebaseConfig config;
    ESPCommunication* espComm;  // Reference to ESP communication
    
    // Device token management
    String deviceTokens[MAX_TOKENS];
    int tokenCount;
    unsigned long lastTokenCheck;
    const unsigned long TOKEN_CHECK_INTERVAL = 300000; // Check tokens every 5 minutes
    
    // Stream configuration
    String tokenParentPath;
    String controlParentPath;
    String deviceId;
    
    // Stream state tracking
    bool controlStreamActive;
    String lastMode;
    int lastFanSpeed;
    
    // Internal methods
    void initializeControlStream();
    void printNetworkDiagnostics();
    void loadTokensFromDatabase();
    
    // Callback functions
    static void controlStreamCallback(MultiPathStream stream);
    static void controlStreamTimeoutCallback(bool timeout);
};

// Global instance for callback access
extern FirebaseManager* globalFirebaseManager;

#endif // FIREBASE_CONFIG_H
