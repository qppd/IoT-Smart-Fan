#ifndef PINCONFIG_H
#define PINCONFIG_H

// Pin definitions for Smart Fan project

// Sensor pins
#define DHT_PIN 4               // DHT22 temperature and humidity sensor pin
#define CURRENT_SENSOR_PIN 34   // ACS712 current sensor pin (analog)
// Note: Voltage sensor (ZMPT101B) removed - using fixed 220V value

// Control pins
#define BUZZER_PIN 25          // Piezo buzzer pin
#define TRIAC_PIN 18           // TRIAC module output pin (RobotDyn compatible)
#define ZERO_CROSS_PIN 5       // Zero cross detection pin for TRIAC dimming (RobotDyn compatible)

// Serial communication pins for inter-board communication
#define ESP_SERIAL_RX 16       // Hardware Serial RX pin for ESP8266 communication
#define ESP_SERIAL_TX 17       // Hardware Serial TX pin for ESP8266 communication

#endif // PINCONFIG_H