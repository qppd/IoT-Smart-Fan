
# SmartFan: IoT Stand Fan Automation & Android App

---


## Recent Updates (September 2025)

- **TRIACModule PWM Integration:**
   - Added `TRIACModule.h` and `TRIACModule.cpp` in `source/esp/SmartFan/` for robust, interrupt-driven PWM-based TRIAC control of the fan.
   - Integrated the new module into `SmartFan.ino` for modular, testable fan speed control.
   - The TRIACModule enables precise, software-based phase angle control of a TRIAC for fan speed, replacing analog dimming logic.
   - Example code in `SmartFan.ino` sweeps TRIAC power from 0% to 100% and back for validation.
   - The implementation is based on the RobotDyn Dimmer library, but fully adapted for ESP32 and modular C++.

Refer to the new files and the updated `SmartFan.ino` for implementation details. The codebase is now highly modular, with each hardware component and logic encapsulated in its own class for maintainability and extension.

SmartFan is a complete IoT solution for automating and monitoring a standard-size stand fan using an ESP32 microcontroller and a companion Android app. It features:

- Automatic fan speed control based on temperature (PID algorithm)
- Real-time temperature, humidity, current, and voltage monitoring
- Buzzer alerts for over-temperature
- Firebase integration for cloud data logging and remote control
- Dynamic Wi-Fi configuration using WiFiManager
- Android app for real-time monitoring, control, notifications, and device management

All hardware abstraction, control logic, and app features are modularized for easy maintenance and extension.

---


## ESP32 Firmware Features

- **Automatic Fan Speed Control**: PID-based adjustment of fan speed according to temperature, with configurable setpoint and PID parameters.
- **TRIAC Phase Angle Control**: Software-based PWM/phase angle control of a TRIAC for universal motor fans, using zero-cross detection and precise timing (see `TRIACModule`).
- **Temperature & Humidity Monitoring**: DHT22 sensor abstraction for environmental data.
- **Current & Voltage Monitoring**: ACS712 and ZMPT101B sensors, with calibration and RMS calculation, for real-time power usage and safety monitoring.
- **Buzzer Alerts**: Audible warning if temperature exceeds setpoint (configurable, see `BUZZERConfig`).
- **Firebase Integration**: Real-time upload of all sensor and control data, plus device logs, to Firebase Realtime Database.
- **Wi-Fi Credentials Management**: User-friendly setup via WiFiManager captive portal (first boot AP mode).
- **Highly Modular Codebase**: Each hardware component and logic is encapsulated in its own C++ class/module for easy maintenance and extension.

---

## Android App Features

- **User Authentication**: Secure login and registration using Firebase Authentication (email/password).
- **Device Linking**: Link your ESP32-powered fan by entering or scanning a Device ID. Each device is associated with a user account in Firebase.
- **Dashboard**: Real-time temperature display (animated gauge), fan status, manual and auto fan speed controls.
- **Settings & Device Management**: Light/dark mode, set temperature thresholds, manage linked devices, account management.
- **History & Logs**: View historical temperature and fan speed logs (last 50 entries).
- **Notifications**: Push notifications via Firebase Cloud Messaging (FCM) for alerts (e.g., temperature threshold exceeded).
- **Security**: Firebase security rules restrict data access to authenticated users and device owners. Input validation throughout the app.

---

## System Architecture

- **ESP32 Device**: Reads temperature, controls fan via TRIAC/VFD, communicates with Firebase.
- **Android App**: Modern Material UI, Firebase integration, real-time updates, device management.
- **Firebase Backend**: Authentication, Realtime Database, Cloud Messaging (for notifications).

---


## Hardware Requirements (ESP32)

- **ESP32 Microcontroller**
- **DHT22 Temperature and Humidity Sensor**
- **ACS712 Current Sensor** (default: 5A, pin 34)
- **ZMPT101B Voltage Sensor** (pin 35)
- **Piezo Buzzer** (pin 25)
- **AC Light Lamp Dimming and Motor Dimmer Module** (TRIAC module, 1 Channel, 3.3V/5V logic, 50/60Hz, 8A-400V)
   - This module controls the amount of AC voltage delivered to a device, as used in fan regulators or light dimmer switches.
   - Suitable for home and industrial use: can dim lights, control AC motors, electric pumps, and tools.
   - Supports up to 8A load current, 400V AC, with overload protection (auto cutoff on overload).
   - Provides smooth dimming (slow ramp up/down) for lamps and fans, creating a pleasant effect.
   - Typical size: ~5.7x2.85cm (2.24x1.12in). Compatible with ESP32 3.3V logic.
   - Example: Robodyn or similar modules.
- **Power Supply**

---

## Software Requirements

- **Arduino IDE** (latest version)
- **Firebase ESP-Client Library** by Mobizt
- **WiFiManager Library**
- **PID_v1 Library**
- **Android Studio** (for app)

---

## Installation

### ESP32 Firmware
1. **Clone the Repository**:
    ```bash
    git clone https://github.com/qppd/IoT-Smart-Fan.git
    ```
2. **Install Required Libraries**:
    - Open Arduino IDE.
    - Go to **Sketch > Include Library > Manage Libraries...**.
    - Install:
       - **Firebase ESP-Client**
       - **WiFiManager**
       - **PID_v1**
3. **Configure Firebase and Wi-Fi**:
    - Edit `firebase_credentials.h` and set your Firebase and Wi-Fi credentials.
4. **Upload the Code**:
    - Connect your ESP32 to your computer.
    - Select the correct board and port in Arduino IDE.
    - Upload the code to the ESP32.

### Android App
1. Open `source/android/SmartFan` in Android Studio.
2. Add your `google-services.json` to the `app/` directory.
3. Build and run on your device.

---


## How It Works

### ESP32
1. **Wi-Fi Configuration**: On first boot, ESP32 creates an access point (`SmartFan_AP`). Connect and enter Wi-Fi credentials via the captive portal (WiFiManager).
2. **Sensor Readings**: DHT22 measures temperature/humidity; ACS712 measures current; ZMPT101B measures voltage (with calibration for true RMS).
3. **Fan Speed Control (PID + TRIAC)**: PID algorithm computes fan speed based on temperature. The `TRIACModule` uses zero-cross detection and phase angle control to set fan power (0–100%).
4. **Buzzer Alerts**: If temperature exceeds setpoint + 2°C, buzzer beeps for 300ms (configurable).
5. **Data Logging (Firebase)**: All sensor readings and fan control values are sent to Firebase RTDB in real-time. Device logs are also created for history.
6. **Serial Monitoring**: All values are printed to the serial console for debugging and monitoring.
7. **Modular Design**: All hardware and logic (sensors, actuators, cloud, control) are implemented as separate C++ classes for clarity and reusability.

### Android App
1. **User registers/logs in.**
2. **Links their ESP32-powered fan** using Device ID.
3. **Dashboard displays real-time temperature** (animated gauge) and fan status.
4. **User can control fan manually or enable auto mode.**
5. **Settings and logs** provide further customization and insights.

---

## File Structure (Key Parts)

```plaintext
model/                        # (reserved for future ML models)
source/
   android/SmartFan/           # Android app project
      app/
         src/main/java/com/qppd/smartfan/
            auth/                 # Login, Register
            device/               # Device linking, management
            ui/                   # Splash, main UI
            ...
         ...
      ...
   esp/SmartFan/               # ESP32 firmware
      SmartFan.ino              # Main application logic
      DHTSensor.cpp/.h          # DHT22 abstraction
      CURRENTSensor.cpp/.h      # ACS712 abstraction
      VOLTAGESensor.cpp/.h      # ZMPT101B abstraction
      PIDConfig.cpp/.h          # PID logic
      BUZZERConfig.cpp/.h       # Buzzer control
      FirebaseConfig.cpp/.h     # Firebase/Wi-Fi setup
      firebase_credentials.h    # Credentials
      PinConfig.h               # Pin assignments
      ...
```

---

## Database Structure (Realtime Database Example)
```json
{
   "users": {
      "uid123": {
         "email": "user@email.com",
         "devices": {
            "deviceIdABC": true
         },
         "settings": {
            "theme": "dark",
            "tempMin": 25,
            "tempMax": 30
         }
      }
   },
   "devices": {
      "deviceIdABC": {
         "owner": "uid123",
         "name": "Living Room Fan",
         "current": {
            "temperature": 28.5,
            "fanSpeed": 2,
            "mode": "auto",
            "lastUpdate": 1692620000
         },
         "logs": {
            "logId1": {
               "timestamp": 1692620000,
               "temperature": 28.5,
               "fanSpeed": 2
            }
         }
      }
   }
}
```

---

## Libraries & Tools
- **Firebase Auth, Database, Messaging**
- **SpeedView** (animated gauge)
- **Material Components**
- **AndroidX**

---

## Future Enhancements
- Device management (rename/unlink)
- Advanced settings (thresholds, schedules)
- Data visualization (charts, history)
- Push notifications
- Multi-device support

---

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request. For major changes, please open an issue first to discuss what you would like to change.

---

## License

ESP32 firmware: MIT License. Android app: Apache 2.0 License. See the `LICENSE` file for details.

---

## Author

Created by [qppd](https://github.com/qppd).
