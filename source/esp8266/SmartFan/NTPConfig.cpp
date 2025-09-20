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