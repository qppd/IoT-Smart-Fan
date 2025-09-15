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


#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <FirebaseClient.h>
#include "FirebaseConfig.h"
#include "firebase_credentials.h"

// --- Microtask 1: Global objects and auth setup ---
SSL_CLIENT ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

UserAuth user_auth(FIREBASE_AUTH, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult databaseResult;
bool firebaseTaskComplete = false;

// --- Microtask 2: Firebase initialization ---
void initFirebase() {
    Serial.println("[Firebase] Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    set_ssl_client_insecure_and_buffer(ssl_client);

    Serial.println("Initializing Firebase app...");
    initializeApp(aClient, app, getAuth(user_auth), auth_debug_print, "🔐 authTask");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(FIREBASE_HOST); // Use host as database URL
}

// --- Microtask 3: Data update example ---
void updateFirebaseData(const String &path, int value) {
    object_t json;
    JsonWriter writer;
    writer.create(json, "data/value", value);
    Serial.println("[Firebase] Updating data (JSON object only)...");
    Database.update(aClient, path.c_str(), json, processData, "updateTask");
}

// --- Microtask 4: Async result processing ---
void processData(AsyncResult &aResult) {
    if (!aResult.isResult())
        return;
    if (aResult.isEvent()) {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
    }
    if (aResult.isDebug()) {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }
    if (aResult.isError()) {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }
    if (aResult.available()) {
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    }
}

// --- Microtask 5: Firebase loop for async tasks ---
void firebaseLoop() {
    app.loop();
    processData(databaseResult);
}
