#ifndef FIREBASECONFIG_H
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
