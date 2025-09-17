#ifndef PINCONFIG_H
#define PINCONFIG_H

// Pin definitions for ESP8266 Smart Fan project
#define DHT_PIN D2
#define CURRENT_SENSOR_PIN A0
#define VOLTAGE_SENSOR_PIN A0
#define BUZZER_PIN D5
#define TRIAC_PIN D1            // Moved from D6 to D1 for serial communication
#define ZERO_CROSS_PIN D8       // Moved from D7 to D8 for serial communication
#define WIFI_RESET_PIN D3

// Serial communication pins for inter-board communication
#define ESP_SERIAL_RX D6        // Software Serial RX pin for ESP32 communication
#define ESP_SERIAL_TX D7        // Software Serial TX pin for ESP32 communication

#endif // PINCONFIG_H
