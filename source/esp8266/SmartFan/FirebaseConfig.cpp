#include "FirebaseConfig.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <time.h>

// Global instance pointer for callback access
FirebaseManager* globalFirebaseManager = nullptr;

FirebaseManager::FirebaseManager() {
    tokenCount = 0;
    tokenParentPath = "smartfan";
    tokenPaths[0] = "/tokens";
    globalFirebaseManager = this;
}

bool FirebaseManager::setupNTPTime() {
    Serial.println("Setting up NTP time synchronization...");
    
    // Configure multiple NTP servers for redundancy
    configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
    
    int retry = 0;
    const int maxRetries = 20;
    const int retryDelay = 1000; // 1 second
    
    Serial.print("Waiting for NTP time sync");
    while (!time(nullptr) && retry < maxRetries) {
        Serial.print(".");
        delay(retryDelay);
        retry++;
    }
    
    if (retry >= maxRetries) {
        Serial.println("\nNTP time sync failed!");
        return false;
    }
    
    Serial.println("\nNTP time synchronized successfully!");
    
    // Print current time for verification
    time_t now = time(nullptr);
    Serial.print("Current time: ");
    Serial.println(ctime(&now));
    
    return true;
}

void FirebaseManager::printNetworkDiagnostics() {
    Serial.println("\n=== Network Diagnostics ===");
    
    // WiFi Status
    Serial.printf("WiFi Status: %s\n", 
                  WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS Server: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("Signal Strength (RSSI): %d dBm\n", WiFi.RSSI());
    
    // DNS Resolution Test
    Serial.println("\nTesting DNS resolution...");
    IPAddress ip;
    if (WiFi.hostByName("pool.ntp.org", ip)) {
        Serial.printf("DNS Test: SUCCESS - pool.ntp.org resolved to %s\n", ip.toString().c_str());
    } else {
        Serial.println("DNS Test: FAILED - Could not resolve pool.ntp.org");
    }
    
    // NTP Time Check
    time_t now = time(nullptr);
    if (now > 1000000000) { // Valid timestamp (after year 2001)
        Serial.printf("NTP Time: SUCCESS - %s", ctime(&now));
    } else {
        Serial.println("NTP Time: FAILED - Time not synchronized");
    }
    
    // Memory Status
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Heap Fragmentation: %d%%\n", ESP.getHeapFragmentation());
    
    Serial.println("=== End Diagnostics ===\n");
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

    // Print network diagnostics for debugging
    printNetworkDiagnostics();

    // Setup NTP time synchronization BEFORE Firebase initialization
    if (!setupNTPTime()) {
        Serial.println("Failed to synchronize time. Firebase authentication may fail.");
        // Print diagnostics again to see if anything changed
        printNetworkDiagnostics();
    }

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    // Assign Firebase credentials
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;

    // Optional: Token status callback (using library's default)
    // config.token_status_callback = tokenStatusCallback;

    // Set network reconnection
    Firebase.reconnectNetwork(true);
    
    // Configure buffer sizes
    fbdo.setBSSLBufferSize(4096, 1024);
    fbdo.setResponseSize(4096);

    // Set timeouts to handle slow connections
    config.timeout.serverResponse = 10 * 1000; // 10 seconds
    config.timeout.socketConnection = 10 * 1000; // 10 seconds
    config.timeout.sslHandshake = 30 * 1000; // 30 seconds
    config.timeout.rtdbKeepAlive = 45 * 1000; // 45 seconds
    config.timeout.rtdbStreamReconnect = 1 * 1000; // 1 second
    config.timeout.rtdbStreamError = 3 * 1000; // 3 seconds

    // Begin Firebase with retry logic
    Serial.println("Initializing Firebase...");
    
    int retryCount = 0;
    const int maxRetries = 5;
    const int baseDelay = 2000; // 2 seconds
    
    while (retryCount < maxRetries) {
        Firebase.begin(&config, &auth);
        
        // Wait for Firebase to initialize with timeout
        int initWait = 0;
        const int maxInitWait = 30; // 30 seconds max wait
        
        while (!Firebase.ready() && initWait < maxInitWait) {
            delay(1000);
            initWait++;
            Serial.print(".");
        }
        
        if (Firebase.ready()) {
            Serial.println("\nFirebase initialized successfully!");
            break;
        } else {
            retryCount++;
            Serial.printf("\nFirebase initialization failed (attempt %d/%d)\n", retryCount, maxRetries);
            
            if (retryCount < maxRetries) {
                int delayTime = baseDelay * (1 << (retryCount - 1)); // Exponential backoff
                Serial.printf("Retrying in %d seconds...\n", delayTime / 1000);
                delay(delayTime);
                
                // Re-sync time before retry
                setupNTPTime();
            }
        }
    }
    
    if (!Firebase.ready()) {
        Serial.println("Failed to initialize Firebase after all retries!");
        return;
    }
    
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
