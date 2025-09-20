#ifndef NTP_CONFIG_H
#define NTP_CONFIG_H

#include <ESP8266WiFi.h>
#include <time.h>

// Global NTP variables
extern String DATETIME;
extern const char* weekDays[7];
extern const char* months[12];

// NTP function declarations
void initNTP();
void getNTPDate();

#endif // NTP_CONFIG_H