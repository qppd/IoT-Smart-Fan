#ifndef PINCONFIG_H
#define PINCONFIG_H

// Pin definitions for Smart Fan project

// Sensor pins
#define DHT_PIN 4               // DHT22 temperature and humidity sensor pin
#define CURRENT_SENSOR_PIN 34   // ACS712 current sensor pin (analog)
#define VOLTAGE_SENSOR_PIN 35   // ZMPT101B voltage sensor pin (analog)

// Control pins
#define BUZZER_PIN 25          // Piezo buzzer pin
#define TRIAC_PIN 12           // TRIAC module output pin
#define ZERO_CROSS_PIN 2       // Zero cross detection pin for TRIAC dimming

#endif // PINCONFIG_H