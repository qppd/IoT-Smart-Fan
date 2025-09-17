#include "FirebaseConfig.h"

FirebaseManager::FirebaseManager() {}

void FirebaseManager::begin() {
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    Serial.println("Firebase initialized.");
}

void FirebaseManager::updateDeviceCurrent(const String& deviceId, float temperature, int fanSpeed, const String& mode, unsigned long lastUpdate, float voltage, float current, float watt, float kwh) {
    String path = "/devices/" + deviceId + "/current";
    FirebaseJson json;
    json.set("temperature", temperature);
    json.set("fanSpeed", fanSpeed);
    json.set("mode", mode);
    json.set("lastUpdate", (int64_t)lastUpdate);
    json.set("voltage", voltage);
    json.set("current", current);
    json.set("watt", watt);
    json.set("kwh", kwh);
    if (Firebase.setJSON(_fbdo, path, json)) {
        Serial.println("Device current state updated in Firebase.");
    } else {
        Serial.print("Failed to update device current: ");
        Serial.println(_fbdo.errorReason());
    }
}

void FirebaseManager::logDeviceData(const String& deviceId, unsigned long timestamp, float temperature, int fanSpeed, float voltage, float current, float watt, float kwh) {
    String logId = String(timestamp);
    String path = "/devices/" + deviceId + "/logs/" + logId;
    FirebaseJson json;
    json.set("timestamp", (int64_t)timestamp);
    json.set("temperature", temperature);
    json.set("fanSpeed", fanSpeed);
    json.set("voltage", voltage);
    json.set("current", current);
    json.set("watt", watt);
    json.set("kwh", kwh);
    if (Firebase.setJSON(_fbdo, path, json)) {
        Serial.println("Device log entry added in Firebase.");
    } else {
        Serial.print("Failed to add device log: ");
        Serial.println(_fbdo.errorReason());
    }
}

void FirebaseManager::resetWiFiSettings() {
    WiFi.disconnect(true);
    ESP.restart();
}

bool FirebaseManager::isReady() {
    return WiFi.status() == WL_CONNECTED;
}
