#define FIREBASECONFIG_H

#include <WiFi.h>
#include <FirebaseESP32.h>
#include "firebase_credentials.h"

// Firebase objects
extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;

// Function to initialize Firebase and Wi-Fi
void initFirebase();

#endif // FIREBASECONFIG_H

#ifndef FIREBASECONFIG_H
#define FIREBASECONFIG_H

#include <WiFi.h>
#include <FirebaseClient.h>
#include "firebase_credentials.h"

// --- Async Firebase objects ---
extern SSL_CLIENT ssl_client;
extern AsyncClientClass aClient;
extern UserAuth user_auth;
extern FirebaseApp app;
extern RealtimeDatabase Database;
extern AsyncResult databaseResult;
extern bool firebaseTaskComplete;

// --- Functions ---
void initFirebase();
void updateFirebaseData(const String &path, int value);
void processData(AsyncResult &aResult);
void firebaseLoop();

#endif // FIREBASECONFIG_H
