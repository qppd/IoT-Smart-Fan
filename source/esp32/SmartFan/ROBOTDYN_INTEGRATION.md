# RobotDyn Dimmer Library Integration

## Overview
The Smart Fan ESP32 project has been updated to use the RobotDyn Dimmer Library for TRIAC-based AC motor speed control. This provides reliable zero-cross detection and phase angle control for smooth fan speed regulation.

## Hardware Requirements
- RobotDyn AC Dimmer Module
- ESP32 Development Board
- AC Fan Motor
- Zero-cross detection circuit (usually included with RobotDyn module)

## Pin Configuration
Based on the RobotDyn library compatibility chart for ESP32:

| Function | GPIO Pin | Description |
|----------|----------|-------------|
| TRIAC Output | GPIO18 | Controls TRIAC gate signal |
| Zero Cross Detection | GPIO5 | Detects AC zero-crossing events |

## Library Installation
Install the RobotDyn Dimmer Library through Arduino IDE:
1. Open Arduino IDE
2. Go to Tools → Manage Libraries
3. Search for "RBDdimmer"
4. Install the library by RobotDyn

Or download from: https://github.com/RobotDynOfficial/RBDDimmer

## Code Changes Made

### TRIACModule.h
- Added RBDdimmer.h include
- Simplified class structure to use dimmerLamp object
- Updated zero cross pin default to GPIO5
- Removed custom ISR implementation (handled by library)

### TRIACModule.cpp
- Replaced custom dimming logic with RobotDyn library calls
- Simplified initialization using dimmer.begin(NORMAL_MODE, ON)
- Direct power control using dimmer.setPower(0-100)
- Added proper library object lifecycle management

### PinConfig.h
- Updated TRIAC_PIN to GPIO18
- Updated ZERO_CROSS_PIN to GPIO5

### SmartFan.ino
- Added testTRIACDimmer() function for testing
- Integrated test into setup() (commented out by default)

## Usage Example

```cpp
#include "TRIACModule.h"

TRIACModule triac(18, 5); // output pin, zero cross pin

void setup() {
    triac.begin();
    triac.setPower(50); // Set to 50% power
}

void loop() {
    // Gradually increase power from 0% to 100%
    for (int power = 0; power <= 100; power += 10) {
        triac.setPower(power);
        delay(1000);
    }
}
```

## Safety Considerations
- Always use proper AC isolation when working with mains voltage
- Test with low-voltage AC sources first
- Ensure proper grounding and safety measures
- The TRIAC module should include optical isolation
- Never work on live AC circuits without proper safety equipment

## Testing
Uncomment the testTRIACDimmer() call in setup() to run a power sweep test:
- Tests power levels: 0%, 25%, 50%, 75%, 100%
- Each level held for 2 seconds
- Returns to 0% power at end
- Serial output shows current power levels

## Troubleshooting
1. **No dimming response**: Check zero-cross pin connection
2. **Erratic behavior**: Verify AC phase detection wiring
3. **Library errors**: Ensure RBDdimmer library is installed
4. **Power level mismatch**: Library may adjust values internally

## References
- RobotDyn Dimmer Library: https://github.com/RobotDynOfficial/RBDDimmer
- ESP32 Pin Reference: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
- AC Dimming Theory: https://www.electronics-tutorials.ws/power/triac.html