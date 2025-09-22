#include "NTPConfig.h"

// Global NTP variables
String DATETIME = "";

// Week Days - Using const char* arrays to save heap memory
const char* weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Month Names - Using const char* arrays to save heap memory  
const char* months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

void initNTP() {
  Serial.println("Initializing NTP time synchronization...");
  
  // Set timezone to GMT+8 (28800 seconds)
  configTime(28800, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync");
  struct tm timeinfo;
  int retries = 0;
  while (!getLocalTime(&timeinfo) && retries < 10) {
    Serial.print(".");
    delay(1000);
    retries++;
  }

  if (retries >= 10) {
    Serial.println("\nFailed to get time from NTP");
  } else {
    Serial.println("\nTime synced from NTP successfully!");
    
    // Print current time for verification
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("Current time: ");
    Serial.println(timeStringBuff);
  }
}

void getNTPDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  DATETIME = String(timeStringBuff);
  Serial.println("Current DateTime: " + DATETIME);
}

unsigned long getNTPTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ Failed to obtain NTP time for timestamp");
    return 0; // Return 0 to indicate failure
  }
  
  time_t timestamp = mktime(&timeinfo);
  return (unsigned long)timestamp;
}

unsigned long getNTPTimestampWithFallback() {
  unsigned long ntpTime = getNTPTimestamp();
  
  if (ntpTime == 0) {
    // NTP failed, use fallback with warning
    Serial.println("⚠️ NTP unavailable, using fallback timestamp");
    unsigned long fallbackTime = millis() / 1000 + 1692620000; // Original fallback method
    return fallbackTime;
  }
  
  // Log successful NTP time retrieval periodically
  static unsigned long lastNTPLog = 0;
  if (millis() - lastNTPLog > 300000) { // Log every 5 minutes
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStringBuff[50];
      strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
      Serial.println("✅ NTP time synchronized: " + String(timeStringBuff));
    }
    lastNTPLog = millis();
  }
  
  return ntpTime;
}

bool isNTPSynced() {
  time_t now = time(nullptr);
  return (now > 1000000000); // Valid timestamp (after year 2001)
}

String getFormattedDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Return fallback formatted time if NTP fails
    unsigned long fallbackTimestamp = millis() / 1000 + 1692620000;
    time_t fallbackTime = (time_t)fallbackTimestamp;
    struct tm* fallbackTimeInfo = localtime(&fallbackTime);
    
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", fallbackTimeInfo);
    return String(timeStringBuff) + " (EST)"; // Estimated time
  }
  
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

String getFormattedDateTimeWithFallback() {
  if (isNTPSynced()) {
    return getFormattedDateTime();
  } else {
    // Use fallback with clear indication
    unsigned long fallbackTimestamp = millis() / 1000 + 1692620000;
    time_t fallbackTime = (time_t)fallbackTimestamp;
    struct tm* fallbackTimeInfo = localtime(&fallbackTime);
    
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", fallbackTimeInfo);
    return String(timeStringBuff) + " (EST)"; // Estimated time
  }
}

String getCurrentLogPrefix() {
  String datetime = getFormattedDateTimeWithFallback();
  return "[" + datetime + "] ";
}