#ifndef TRIACMODULE_H
#define TRIACMODULE_H

#include <Arduino.h>
// Commented out problematic RBDdimmer library
// #include <RBDdimmer.h>  // RBD Dimmer library for TRIAC control

class TRIACModule {
public:
    TRIACModule(uint8_t outputPin, uint8_t zeroCrossPin = 2);
    void begin();
    void setPower(uint8_t percent); // percent: 0-100
    uint8_t getPower();
    void turnOn();
    void turnOff();
    
private:
    static void IRAM_ATTR zeroCrossISR();
    void IRAM_ATTR handleZeroCross();
    void calculateTriggerDelay(uint8_t power);
    
    uint8_t _outputPin;
    uint8_t _zeroCrossPin;
    uint8_t _currentPower;
    bool _isOn;
    
    volatile bool _zeroCrossDetected;
    volatile uint32_t _triggerDelay; // in microseconds
    
    static TRIACModule* _instance; // For ISR access
    
    // TRIAC control constants
    static const uint32_t HALF_CYCLE_TIME = 10000; // 10ms for 50Hz (8333 for 60Hz)
    static const uint32_t MIN_TRIGGER_DELAY = 500;  // Minimum delay in microseconds
};

#endif // TRIACMODULE_H
