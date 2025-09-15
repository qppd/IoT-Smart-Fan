# WiFiManager Library Installation Instructions

## Required Libraries for ESP32 WiFiManager Integration

1. **WiFiManager by tzapu**
   - Repository: https://github.com/tzapu/WiFiManager
   - Arduino IDE: Tools > Manage Libraries > Search "WiFiManager" by tzapu
   - PlatformIO: lib_deps = tzapu/WiFiManager@^0.16.0

2. **ArduinoJson** (dependency for WiFiManager)
   - Arduino IDE: Tools > Manage Libraries > Search "ArduinoJson" by Benoit Blanchon
   - PlatformIO: lib_deps = bblanchon/ArduinoJson@^6.21.3

## Installation Steps:

### Arduino IDE:
1. Open Arduino IDE
2. Go to Tools > Manage Libraries
3. Search for "WiFiManager" and install the library by tzapu
4. Search for "ArduinoJson" and install the library by Benoit Blanchon
5. Restart Arduino IDE

### PlatformIO:
Add to your platformio.ini file:
```
lib_deps = 
    tzapu/WiFiManager@^0.16.0
    bblanchon/ArduinoJson@^6.21.3
```

## Features Added:

1. **Automatic WiFi Connection**: ESP32 will attempt to connect to previously saved WiFi credentials
2. **Captive Portal**: If no saved credentials or connection fails, ESP32 creates an access point
3. **WiFi Reset Button**: Hold BOOT button for 5 seconds to reset WiFi settings
4. **Custom Device ID**: Can be configured through the WiFi setup portal
5. **Non-blocking Operation**: WiFi setup doesn't interfere with existing sensor and control functionality

## Usage:

1. On first boot, ESP32 will create a WiFi access point named "SmartFan-[ChipID]"
2. Connect to this network with password "smartfan123"
3. A captive portal will open automatically, or navigate to 192.168.4.1
4. Enter your WiFi credentials and device settings
5. ESP32 will save settings and connect to your WiFi network
6. Future boots will automatically connect to the saved network

## Reset WiFi Settings:

- Hold the BOOT button (GPIO 0) for 5 seconds
- ESP32 will beep and restart in configuration mode
- Follow the setup process again to configure new WiFi credentials