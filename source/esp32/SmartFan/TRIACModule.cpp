#include "TRIACModule.h"

// Static member initialization
TRIACModule* TRIACModule::_instance = nullptr;

TRIACModule::TRIACModule(uint8_t outputPin, uint8_t zeroCrossPin)
    : _outputPin(outputPin), _zeroCrossPin(zeroCrossPin), _currentPower(0), 
      _isOn(false), _zeroCrossDetected(false), _triggerDelay(HALF_CYCLE_TIME) {
    _instance = this;
}

void TRIACModule::begin() {
    // Configure pins
    pinMode(_outputPin, OUTPUT);
    pinMode(_zeroCrossPin, INPUT_PULLUP); // Use pull-up to avoid floating

    // Initialize output low
    digitalWrite(_outputPin, LOW);

    // Attach zero-cross interrupt
    attachInterrupt(digitalPinToInterrupt(_zeroCrossPin), zeroCrossISR, RISING);

    _isOn = true;
    setPower(0); // Start with 0% power

    // DEBUG: Simulate a zero-cross event at startup to kickstart TRIAC
    Serial.println("[TRIACModule] Simulating zero-cross event at startup");
    handleZeroCross();
}

void TRIACModule::setPower(uint8_t percent) {
    if (percent > 100) percent = 100;
    _currentPower = percent;
    calculateTriggerDelay(percent);
}

uint8_t TRIACModule::getPower() {
    return _currentPower;
}

void TRIACModule::turnOn() {
    _isOn = true;
    calculateTriggerDelay(_currentPower);
}

void TRIACModule::turnOff() {
    _isOn = false;
    _triggerDelay = HALF_CYCLE_TIME; // No trigger
    digitalWrite(_outputPin, LOW);
}

void IRAM_ATTR TRIACModule::zeroCrossISR() {
    if (_instance != nullptr) {
        // DEBUG: Indicate ISR was triggered
        Serial.println("[TRIACModule] Zero-cross ISR triggered");
        _instance->handleZeroCross();
    }
}

void IRAM_ATTR TRIACModule::handleZeroCross() {
    if (!_isOn || _currentPower == 0) {
        digitalWrite(_outputPin, LOW);
        return;
    }
    
    if (_currentPower >= 100) {
        // Full power - trigger immediately
        digitalWrite(_outputPin, HIGH);
        delayMicroseconds(10); // Short pulse
        digitalWrite(_outputPin, LOW);
        return;
    }
    
    // Schedule trigger after calculated delay
    _zeroCrossDetected = true;
    
    // Use timer for precise delay
    delayMicroseconds(_triggerDelay);
    
    // Trigger TRIAC
    digitalWrite(_outputPin, HIGH);
    delayMicroseconds(10); // Short pulse to trigger TRIAC
    digitalWrite(_outputPin, LOW);
}

void TRIACModule::calculateTriggerDelay(uint8_t power) {
    if (power == 0) {
        _triggerDelay = HALF_CYCLE_TIME;
        return;
    }
    
    if (power >= 100) {
        _triggerDelay = MIN_TRIGGER_DELAY;
        return;
    }
    
    // Calculate delay based on power percentage
    // Lower power = longer delay (trigger later in the cycle)
    // Formula: delay = (100 - power) * (HALF_CYCLE_TIME - MIN_TRIGGER_DELAY) / 100 + MIN_TRIGGER_DELAY
    _triggerDelay = ((100 - power) * (HALF_CYCLE_TIME - MIN_TRIGGER_DELAY)) / 100 + MIN_TRIGGER_DELAY;
}
