#include "FirebaseConfig.h"
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

FirebaseManager::FirebaseManager() : _tokenCount(0), _tokenParentPath("plastech") {
    _instance = this;
}

void FirebaseManager::begin() {
    initWiFi();
    
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    _config.api_key = API_KEY;
    _config.database_url = DATABASE_URL;

    _config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    _config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    _config.service_account.data.private_key = PRIVATE_KEY;

    Firebase.reconnectNetwork(true);
    _fbdo.setBSSLBufferSize(4096, 1024);
    _fbdo.setResponseSize(4096);

    Firebase.begin(&_config, &_auth);
    while (!Firebase.ready()) {
        delay(100);
    }
}

void FirebaseManager::initWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
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
    if (Firebase.RTDB.setJSON(&_fbdo, path, &json)) {
        Serial.println("Device current state updated in Firebase.");
    } else {
        Serial.print("Failed to update device current: ");
        Serial.println(_fbdo.errorReason());
    }
}

void FirebaseManager::logDeviceData(const String& deviceId, unsigned long timestamp, float temperature, int fanSpeed, float voltage, float current, float watt, float kwh) {
    String logId = String(timestamp); // Use timestamp as logId
    String path = "/devices/" + deviceId + "/logs/" + logId;
    FirebaseJson json;
    json.set("timestamp", (int64_t)timestamp);
    json.set("temperature", temperature);
    json.set("fanSpeed", fanSpeed);
    json.set("voltage", voltage);
    json.set("current", current);
    json.set("watt", watt);
    json.set("kwh", kwh);
    if (Firebase.RTDB.setJSON(&_fbdo, path, &json)) {
        Serial.println("Device log entry added in Firebase.");
    } else {
        Serial.print("Failed to add device log: ");
        Serial.println(_fbdo.errorReason());
    }
}

void FirebaseManager::sendMessageToAll(const String& title, const String& body) {
    for (int i = 0; i < _tokenCount; i++) {
        Serial.println("📲 Sending to: " + _deviceTokens[i]);
        FCM_HTTPv1_JSON_Message msg;
        msg.token = _deviceTokens[i];
        msg.notification.title = title;
        msg.notification.body = body;
        FirebaseJson payload;
        payload.add("status", "1");
        msg.data = payload.raw();
        if (Firebase.FCM.send(&_fbdo, &msg)) {
            Serial.println("Sent!");
        } else {
            Serial.println("Error: " + _fbdo.errorReason());
        }
    }
}

void FirebaseManager::tokenStreamCallback(MultiPathStream stream) {
    if (_instance && stream.get("/tokens")) {
        Serial.println("token Updated Path: " + stream.dataPath);
        Serial.println("token New Value: " + stream.value);
        FirebaseJson json;
        FirebaseJsonData result;
        json.clear();
        json.setJsonData(stream.value);
        size_t count = json.iteratorBegin();
        _instance->_tokenCount = 0;
        for (size_t i = 0; i < count; i++) {
            FirebaseJson::IteratorValue value = json.valueAt(i);
            String pushID = value.key;
            if (!value.value.startsWith("{")) {
                Serial.println("Ignored non-object key: " + pushID);
                continue;
            }
            String fullPath = pushID + "/device_token";
            if (json.get(result, fullPath)) {
                String deviceToken = result.stringValue;
                Serial.println("✅ Extracted device_token from " + pushID + ": " + deviceToken);
                if (_instance->_tokenCount < MAX_TOKENS) {
                    _instance->_deviceTokens[_instance->_tokenCount] = deviceToken;
                    _instance->_tokenCount++;
                    Serial.println("Adding to array");
                } else {
                    Serial.println("⚠️ Token list full, cannot store more.");
                }
            } else {
                Serial.println("⚠️ Skipping key (no device_token): " + pushID);
            }
        }
        json.iteratorEnd();
        _instance->sendMessageToAll("PlasTech", "This is plastech first notification! Hi :)");
    }
}

void FirebaseManager::tokenStreamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("token stream timed out, attempting to resume...");
    }
    if (_instance && !_instance->_tokenStream.httpConnected()) {
        Serial.printf("token Error code: %d, reason: %s\n", _instance->_tokenStream.httpCode(), _instance->_tokenStream.errorReason().c_str());
    }
}

void FirebaseManager::beginTokenStream() {
    Serial.println("Starting token stream");
    if (!Firebase.RTDB.beginMultiPathStream(&_tokenStream, _tokenParentPath)) {
        Serial.printf("token stream initialization failed: %s\n", _tokenStream.errorReason().c_str());
    } else {
        Firebase.RTDB.setMultiPathStreamCallback(&_tokenStream, tokenStreamCallback, tokenStreamTimeoutCallback);
        Serial.println("Firebase token stream initialized successfully!");
    }
}

bool FirebaseManager::isReady() {
    return Firebase.ready();
}

// Static instance initialization
FirebaseManager* FirebaseManager::_instance = nullptr;
