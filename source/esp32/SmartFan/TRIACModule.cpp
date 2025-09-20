/**
 * TRIACModule.cpp
 * 
 * Implementation of TRIAC control using RobotDyn Dimmer Library
 * 
 * The RobotDyn dimmer library handles:
 * - Zero-cross detection interrupts
 * - Phase angle calculation
 * - TRIAC gate triggering
 * - Power percentage to phase angle conversion
 */

#include "TRIACModule.h"

TRIACModule::TRIACModule(uint8_t outputPin, uint8_t zeroCrossPin)
    : _outputPin(outputPin), _zeroCrossPin(zeroCrossPin), _currentPower(0), 
      _isOn(false), _dimmer(nullptr) {
}

void TRIACModule::begin() {
    // Initialize the RobotDyn dimmer with the specified pins
    _dimmer = new dimmerLamp(_outputPin, _zeroCrossPin);
    
    // Initialize dimmer in NORMAL_MODE and turn it ON
    _dimmer->begin(NORMAL_MODE, ON);
    
    // Set initial power to 0%
    _dimmer->setPower(0);
    _currentPower = 0;
    _isOn = true;
    
    Serial.println("[TRIACModule] Initialized with RobotDyn Dimmer Library");
    Serial.print("[TRIACModule] Output Pin: ");
    Serial.print(_outputPin);
    Serial.print(", Zero Cross Pin: ");
    Serial.println(_zeroCrossPin);
}

void TRIACModule::setPower(uint8_t percent) {
    if (percent > 100) percent = 100;
    
    _currentPower = percent;
    
    if (_dimmer != nullptr && _isOn) {
        _dimmer->setPower(percent);
        Serial.print("[TRIACModule] Power set to: ");
        Serial.print(percent);
        Serial.print("%, Actual dimmer power: ");
        Serial.println(_dimmer->getPower());
    }
}

uint8_t TRIACModule::getPower() {
    if (_dimmer != nullptr) {
        return _dimmer->getPower();
    }
    return _currentPower;
}

void TRIACModule::turnOn() {
    _isOn = true;
    if (_dimmer != nullptr) {
        _dimmer->setState(ON);
        _dimmer->setPower(_currentPower);
        Serial.println("[TRIACModule] Turned ON");
    }
}

void TRIACModule::turnOff() {
    _isOn = false;
    if (_dimmer != nullptr) {
        _dimmer->setPower(0);
        _dimmer->setState(OFF);
        Serial.println("[TRIACModule] Turned OFF");
    }
}
