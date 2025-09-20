/**
 * TRIACModule.h
 * 
 * ESP32 TRIAC Module using RobotDyn Dimmer Library
 * 
 * This module provides AC dimming control using the RobotDyn TRIAC
 * dimmer module. It uses the RBDdimmer library for zero-cross detection
 * and phase angle control.
 * 
 * Pin Configuration for ESP32:
 * - OUTPUT Pin: GPIO18 (connected to TRIAC module input)
 * - ZERO CROSS Pin: GPIO5 (connected to zero-cross detection circuit)
 * 
 * Features:
 * - Power control from 0-100%
 * - Automatic zero-cross detection
 * - Safe turn on/off functionality
 * - Compatible with RobotDyn AC Dimmer Module
 * 
 * Based on RobotDyn Dimmer Library example code
 */

#ifndef TRIACMODULE_H
#define TRIACMODULE_H

#include <Arduino.h>
#include <RBDdimmer.h>  // RBD Dimmer library for TRIAC control

class TRIACModule {
public:
    TRIACModule(uint8_t outputPin, uint8_t zeroCrossPin = 5);
    void begin();
    void setPower(uint8_t percent); // percent: 0-100
    uint8_t getPower();
    void turnOn();
    void turnOff();
    
private:
    uint8_t _outputPin;
    uint8_t _zeroCrossPin;
    uint8_t _currentPower;
    bool _isOn;
    
    // RobotDyn Dimmer object
    dimmerLamp* _dimmer;
};

#endif // TRIACMODULE_H
