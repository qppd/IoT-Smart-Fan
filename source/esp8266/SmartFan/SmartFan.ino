
#include <Arduino.h>
#include "PinConfig.h"
#include "FirebaseConfig.h"
#include "firebase_credentials.h"
#include "WiFiManager.h"

FirebaseManager firebaseManager;

void setup() {
    Serial.begin(115200);
    pinMode(WIFI_RESET_PIN, INPUT_PULLUP);
    setupWiFi();
    firebaseManager.begin();
    // ...initialize other modules as needed...
    Serial.println("ESP8266 Smart Fan Initialized");
}

void loop() {
    // Example: update device state
    String deviceId = "deviceIdESP8266";
    float temperature = 25.0;
    int fanSpeed = 128;
    String mode = "auto";
    unsigned long now = millis() / 1000 + 1692620000;
    float voltage = 220.0;
    float current = 0.5;
    float watt = voltage * current;
    float kwh = 0.01;
    firebaseManager.updateDeviceCurrent(deviceId, temperature, fanSpeed, mode, now, voltage, current, watt, kwh);
    firebaseManager.logDeviceData(deviceId, now, temperature, fanSpeed, voltage, current, watt, kwh);
    delay(2000);
}
