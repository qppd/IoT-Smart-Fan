# Configuration Setup Guide

## Firebase Credentials Setup (ESP8266)

1. Copy `firebase_credentials.h.sample` to `firebase_credentials.h`
2. Replace the placeholder values with your actual Firebase project details:
   - `FIREBASE_HOST`: Your Firebase Realtime Database URL
   - `FIREBASE_AUTH`: Your Firebase authentication token
   - `WIFI_SSID`: Your WiFi network name
   - `WIFI_PASSWORD`: Your WiFi password
   - `DEVICE_ID`: Unique identifier for your device

## Google Services Setup (Android)

1. Copy `google-services.json.sample` to `google-services.json`
2. Replace with the actual `google-services.json` file downloaded from your Firebase project console
   - Go to Firebase Console > Project Settings > General
   - Download the `google-services.json` file for your Android app
   - Replace the sample file with the downloaded file

## Security Note

⚠️ **Important**: Never commit the actual credential files (`firebase_credentials.h` and `google-services.json`) to version control. They are already included in `.gitignore` to prevent accidental commits.

## Getting Firebase Credentials

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Select your project (or create a new one)
3. For ESP8266:
   - Go to Project Settings > Service Accounts
   - Generate a new private key or use database secrets
4. For Android:
   - Go to Project Settings > General
   - Add your Android app if not already added
   - Download the `google-services.json` file