#include "WiFiManager.h"

void setupWiFi() {
    WiFiManager wifiManager;
    // Try to connect to saved WiFi, else start AP for config
    if (!wifiManager.autoConnect("SmartFan-ESP8266", "smartfan123")) {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
        delay(5000);
    }
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}
