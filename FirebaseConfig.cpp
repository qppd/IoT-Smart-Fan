#include "FirebaseConfig.h"
#include <WiFiManager.h>

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void initWiFi() {
    WiFiManager wifiManager;

    // Uncomment to reset WiFi credentials
    // wifiManager.resetSettings();

    // AutoConnect with saved credentials or open portal for new credentials
    if (!wifiManager.autoConnect("SmartFan_AP", "password")) {
        Serial.println("Failed to connect to WiFi. Restarting...");
        delay(3000);
        ESP.restart();
    }

    Serial.println("Connected to WiFi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void initFirebase() {
    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    // Configure Firebase
    config.host = FIREBASE_HOST;
    config.api_key = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("Firebase initialized.");
}
