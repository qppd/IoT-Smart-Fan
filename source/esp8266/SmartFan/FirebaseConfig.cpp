#include "FirebaseConfig.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Global instance pointer for callback access
FirebaseManager* globalFirebaseManager = nullptr;

FirebaseManager::FirebaseManager() {
    tokenCount = 0;
    tokenParentPath = "plastech";
    tokenPaths[0] = "/tokens";
    globalFirebaseManager = this;
}

void FirebaseManager::begin() {
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
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    // Assign Firebase credentials
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    // Optional: Token status callback
    config.token_status_callback = tokenStatusCallback;

    // Recommended settings
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024);
    fbdo.setResponseSize(4096);

    // Begin Firebase
    Firebase.begin(&config, &auth);

    // Wait until Firebase is ready
    while (!Firebase.ready()) {
        delay(100);
    }

    Serial.println("Firebase initialized successfully!");
    
    // Initialize token stream
    initializeTokenStream();
}

void FirebaseManager::initializeTokenStream() {
    Serial.println("Starting token stream");
    if (!Firebase.RTDB.beginMultiPathStream(&tokenStream, tokenParentPath)) {
        Serial.printf("Token stream initialization failed: %s\n", tokenStream.errorReason().c_str());
    } else {
        Firebase.RTDB.setMultiPathStreamCallback(&tokenStream, tokenStreamCallback, tokenStreamTimeoutCallback);
        Serial.println("Firebase token stream initialized successfully!");
    }
}

void FirebaseManager::tokenStreamCallback(MultiPathStream stream) {
    if (globalFirebaseManager == nullptr) return;
    
    // Check if the stream updated /tokens
    if (stream.get("/tokens")) {
        Serial.println("Token Updated Path: " + stream.dataPath);
        Serial.println("Token New Value: " + stream.value);

        FirebaseJson json;
        FirebaseJsonData result;

        // Clear any previous JSON content before loading new data
        json.clear();
        json.setJsonData(stream.value);

        // Begin iterating through all children under /tokens
        size_t count = json.iteratorBegin();
        globalFirebaseManager->tokenCount = 0;  // reset token count before filling

        for (size_t i = 0; i < count; i++) {
            FirebaseJson::IteratorValue value = json.valueAt(i);
            String pushID = value.key;

            // Skip any key that doesn't contain a nested object
            if (!value.value.startsWith("{")) {
                Serial.println("Ignored non-object key: " + pushID);
                continue;
            }

            String fullPath = pushID + "/device_token";

            if (json.get(result, fullPath)) {
                String deviceToken = result.stringValue;
                Serial.println("✅ Extracted device_token from " + pushID + ": " + deviceToken);

                // Add to array if not full
                if (globalFirebaseManager->tokenCount < MAX_TOKENS) {
                    globalFirebaseManager->deviceTokens[globalFirebaseManager->tokenCount] = deviceToken;
                    globalFirebaseManager->tokenCount++;
                    Serial.println("Added to token array, total tokens: " + String(globalFirebaseManager->tokenCount));
                } else {
                    Serial.println("⚠️  Token list full, cannot store more.");
                }
            } else {
                Serial.println("⚠️  Skipping key (no device_token): " + pushID);
            }
        }

        // End iteration
        json.iteratorEnd();

        // Send welcome notification
        globalFirebaseManager->sendMessageToAll("Smart Fan", "Smart Fan system connected! 🌟");
    }
}

void FirebaseManager::tokenStreamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("Token stream timed out, attempting to resume...");
    }
    if (globalFirebaseManager && !globalFirebaseManager->tokenStream.httpConnected()) {
        Serial.printf("Token Error code: %d, reason: %s\n", 
                      globalFirebaseManager->tokenStream.httpCode(), 
                      globalFirebaseManager->tokenStream.errorReason().c_str());
    }
}

void FirebaseManager::sendMessageToAll(String title, String body) {
    for (int i = 0; i < tokenCount; i++) {
        Serial.println("📲 Sending to: " + deviceTokens[i]);

        FCM_HTTPv1_JSON_Message msg;
        msg.token = deviceTokens[i];
        msg.notification.title = title;
        msg.notification.body = body;

        FirebaseJson payload;
        payload.add("status", "1");
        payload.add("device", "smartfan");
        msg.data = payload.raw();

        if (Firebase.FCM.send(&fbdo, &msg)) {
            Serial.println("Notification sent successfully!");
        } else {
            Serial.println("Error sending notification: " + fbdo.errorReason());
        }
    }
}

void FirebaseManager::sendMessage(String title, String body) {
    Serial.print("Send Firebase Cloud Messaging... ");

    FCM_HTTPv1_JSON_Message msg;
    msg.token = DEVICE_REGISTRATION_ID_TOKEN;

    msg.notification.title = title;
    msg.notification.body = body;

    FirebaseJson payload;
    payload.add("status", "1");
    payload.add("device", "smartfan");

    msg.data = payload.raw();

    if (Firebase.FCM.send(&fbdo, &msg)) {
        Serial.printf("FCM sent successfully!\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    } else {
        Serial.println("FCM Error: " + fbdo.errorReason());
    }
}

void FirebaseManager::updateDeviceCurrent(const String& deviceId, float temperature, int fanSpeed, const String& mode, unsigned long lastUpdate, float voltage, float current, float watt, float kwh) {
    String path = "/smartfan/devices/" + deviceId + "/current";
    FirebaseJson json;
    json.set("temperature", temperature);
    json.set("fanSpeed", fanSpeed);
    json.set("mode", mode);
    json.set("lastUpdate", (int64_t)lastUpdate);
    json.set("voltage", voltage);
    json.set("current", current);
    json.set("watt", watt);
    json.set("kwh", kwh);
    
    if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
        Serial.println("Device current state updated in Firebase.");
    } else {
        Serial.print("Failed to update device current: ");
        Serial.println(fbdo.errorReason());
    }
}

void FirebaseManager::logDeviceData(const String& deviceId, unsigned long timestamp, float temperature, int fanSpeed, float voltage, float current, float watt, float kwh) {
    String logId = String(timestamp);
    String path = "/smartfan/devices/" + deviceId + "/logs/" + logId;
    FirebaseJson json;
    json.set("timestamp", (int64_t)timestamp);
    json.set("temperature", temperature);
    json.set("fanSpeed", fanSpeed);
    json.set("voltage", voltage);
    json.set("current", current);
    json.set("watt", watt);
    json.set("kwh", kwh);
    
    if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
        Serial.println("Device log entry added in Firebase.");
    } else {
        Serial.print("Failed to add device log: ");
        Serial.println(fbdo.errorReason());
    }
}

void FirebaseManager::sendSmartFanData(float temperature, int fanSpeed, float voltage, float current, float watt, float kwh) {
    // Create JSON object for Smart Fan data
    FirebaseJson json;
    json.set("temperature", temperature);
    json.set("fanSpeed", fanSpeed);
    json.set("voltage", voltage);
    json.set("current", current);
    json.set("watt", watt);
    json.set("kwh", kwh);
    json.set("device_type", "smart_fan");
    json.set("timestamp", millis());

    Serial.println("Sending Smart Fan data as JSON...");

    // Send to Firebase under smartfan path
    if (Firebase.RTDB.setJSON(&fbdo, "/smartfan/data", &json)) {
        Serial.println("Smart Fan data sent successfully!");
    } else {
        Serial.print("Error sending Smart Fan data: ");
        Serial.println(fbdo.errorReason());
    }
}

void FirebaseManager::resetWiFiSettings() {
    WiFi.disconnect(true);
    ESP.restart();
}

bool FirebaseManager::isReady() {
    return WiFi.status() == WL_CONNECTED && Firebase.ready();
}
