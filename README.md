

# 🌪️ SmartFan: IoT Stand Fan Automation & Android App

<div align="center">

![SmartFan Banner](https://img.shields.io/badge/IoT-Smart%20Fan-blue?style=for-the-badge&logo=wifi)
![ESP32](https://img.shields.io/badge/ESP32-Compatible-green?style=for-the-badge&logo=espressif)
![Android](https://img.shields.io/badge/Android-App-brightgreen?style=for-the-badge&logo=android)
![Firebase](https://img.shields.io/badge/Firebase-Powered-orange?style=for-the-badge&logo=firebase)


**🔥 Complete IoT solution for intelligent fan automation, real-time monitoring, and mobile control**

[📱 Features](#-features) • [⚡ Quick Start](#-quick-start) • [🛠️ Installation](#️-installation) • [📖 Documentation](#-documentation)

</div>

---

## 🎯 Overview


**SmartFan** is a comprehensive IoT ecosystem that transforms any standard stand fan into an intelligent, automated climate control system. Built with a **dual-ESP architecture** (ESP8266 + ESP32) and a modern Android app, it delivers seamless temperature-based automation, real-time monitoring, and cloud integration.


### 🏗️ Dual-ESP Architecture

- **🌐 ESP8266**: WiFi Manager + Firebase Handler — Manages WiFi, cloud database, and push notifications
- **🧠 ESP32**: Sensor Reader + Hardware Controller — Handles DHT22, ACS712, TRIAC fan control, and safety monitoring (220V fixed voltage)
- **📡 Serial Communication**: Custom protocol for real-time data exchange between microcontrollers at 9600 baud (ESP8266: SoftwareSerial, ESP32: HardwareSerial2)


### 🔥 Key Highlights

- **🤖 Intelligent Automation**: Temperature-based fan speed control (auto/manual)
- **📊 Real-time Monitoring**: Live temperature, humidity, voltage, current, power, and energy tracking
- **🏗️ Modular Dual-ESP**: ESP8266 (WiFi/Firebase) + ESP32 (Sensors/Hardware)
- **📡 Robust Serial Protocol**: Reliable, fault-tolerant inter-ESP communication
- **📱 Modern Android App**: Material Design 3, animated gauges, power analytics, device management
- **☁️ Cloud Integration**: Firebase for data logging, user authentication, remote control, and push notifications
- **🔧 Easy Setup**: WiFiManager captive portal, app-based WiFi config, and hardware reset
- **⚡ Power Monitoring**: ACS712 (current) with fixed 220V for calculations, safety alerts and analytics
- **🔔 Smart Notifications**: Push notifications for temperature/power alerts and system status

---



## 📋 Recent Updates (September 2025)

### ⚡ Hardware Simplification: Voltage Sensor Removal
- **ZMPT101B Voltage Sensor**: Removed from hardware design, using fixed 220V value for power calculations
- **GPIO35 Freed**: Pin now available for future expansion or additional sensors
- **Simplified Wiring**: Reduced component count and power consumption
- **Maintained Functionality**: Power monitoring still accurate with current sensor + fixed voltage

### ✅ Major System Milestone: All Core Features Working
- **Serial Communication (ESP8266 ↔ ESP32)**: Fully operational, robust, and reliable at 9600 baud. Real-time data and command exchange confirmed.
- **DHT22 Sensor**: Stable temperature and humidity readings, integrated with automation logic.
- **TRIAC Fan Control**: Precision phase angle control (RobotDyn library), 0–100% sweep validated, zero-cross detection stable.
- **Buzzer Alerts**: Over-temperature and remote-triggered alerts working as intended.

### 🔧 TRIACModule PWM Integration
- ✨ **New Hardware Control**: `TRIACModule` for precision phase angle control (universal motor fans)
- 🧩 **Modular C++**: Clean, testable, and maintainable design
- 📈 **Performance**: Power sweep 0–100% validated, interrupt-driven zero-cross detection
- 🔄 **RobotDyn Compatible**: Adapted for ESP32, modular and extensible

### 📶 WiFi Management Enhancements
- 🌐 **WiFiManager**: Captive portal for first-time setup and reconfiguration
- 📱 **App WiFi Setup**: Configure device WiFi via Android app
- 🔄 **Hardware Reset**: Button for WiFi reset (hold 3s)

### ⚡ Power Monitoring System
- 📊 **Comprehensive Monitoring**: Real-time voltage, current, power (W), and energy (kWh)
- 🚨 **Safety Alerts**: Notifications for high power, overcurrent, and abnormal voltage
- 📈 **Analytics**: kWh tracking, historical logs, and color-coded UI

---


## 📡 Dual-ESP Communication Architecture



### 🏗️ System Architecture Overview

The SmartFan system utilizes a specialized dual-ESP architecture where two microcontrollers work together via serial communication to provide robust, fault-tolerant operation. **As of September 2025, serial communication, DHT22, TRIAC, and buzzer modules are all fully functional and validated in production.**


**ESP8266 (WiFi & Firebase Module):**
- WiFi connection management (captive portal, app-based config)
- Firebase database operations and real-time sync
- Push notifications via Firebase Cloud Messaging
- Data logging, analytics, and remote command handling
- WiFi reset (hardware button, 3s hold)


**ESP32 (Sensor & Hardware Module):**
- DHT22: Temperature/humidity sensor (**fully working**)
- ACS712: Current sensor (RMS, calibrated)
- Fixed 220V for power calculations (voltage sensor removed)
- TRIAC: Fan speed control (phase angle, PWM) (**fully working**)
- Piezo buzzer: Alerts for over-temperature (**fully working**)
- Automatic/manual fan control logic
- Hardware safety monitoring and error detection


### 🔌 Hardware Communication Setup


**Serial Connection:**
- **Baud Rate**: 9600 bps (**confirmed stable**)
- **ESP8266**: D6 (RX), D7 (TX) — SoftwareSerial
- **ESP32**: GPIO16 (RX), GPIO17 (TX) — HardwareSerial2
- **Wiring**: ESP8266 TX (D7) → ESP32 RX (GPIO16), ESP8266 RX (D6) → ESP32 TX (GPIO17)
- **Power**: Separate 3.3V/5V supplies, common ground


### 📨 Communication Protocol


**Message Format:** `<TYPE:DATA>` (delimited, robust parsing, **validated in production**)


**ESP32 → ESP8266 (Sensor Data):**
```
<TEMP:25.5>     // Temperature (°C)
<HUMID:65.0>    // Humidity (%)  
<VOLT:220.0>    // Voltage (V)
<CURR:0.8>      // Current (A)
<FAN:75>        // Fan speed (0-100%)
<BUZZ:ON>       // Buzzer status (now fully implemented)
<STATUS:RUNNING> // System status
<ALL:25.5,65.0,220.0,0.8,75> // Combined data
```


**ESP8266 → ESP32 (Commands):**
```
<CMD:GET_SENSORS>     // Request sensor readings
<CMD:GET_STATUS>      // Health check
<SET_FAN:50>          // Set fan speed
<SET_TEMP:28.0>       // Set target temperature
<FIREBASE:CONNECTED>  // Firebase status
<WIFI:CONNECTED>      // WiFi status
<BUZZ:ALERT>          // Trigger buzzer (now fully implemented)
```


### 🔄 Data Flow & Timing


**Normal Operation Cycle:**
1. **ESP32**: Reads sensors every 2s, sends data every 5s (**DHT, TRIAC, buzzer all validated**)
2. **ESP8266**: Syncs to Firebase every 5s, sends notifications every 30s, requests data every 3s


**Remote Command Flow:**
1. **App/Firebase** → **ESP8266**: Receives commands
2. **ESP8266** → **ESP32**: Forwards hardware commands (serial comms fully working)
3. **ESP32**: Executes, confirms, and reports (DHT, TRIAC, buzzer all working)
4. **ESP8266**: Logs to Firebase


### 🛡️ Safety & Error Handling


**Communication Safeguards:**
- Timeout detection (1s, retry logic)
- Connection health checks (ping/pong)
- Fallback: ESP32 runs autonomously if comms lost
- Error alerts: ESP8266 notifies Firebase
- **All communication and error handling paths validated in production**


**Sensor Validation:**
- Range checking, NaN filtering
- ESP32 sends error status to ESP8266
- Safe fallback values on error
- **DHT, TRIAC, and buzzer error handling confirmed**

---


## 🚀 Features


### 🧠 ESP32 Hardware Controller


<details>
<summary><b>🌡️ Intelligent Climate Control</b></summary>

- **🎯 Temperature-Based Control**: Automatic fan speed adjustment based on DHT22 temperature readings
- **🔧 TRIAC Phase Control**: Precision PWM/phase angle control for universal motor fans (0-100%)
- **📊 Environmental Monitoring**: DHT22 sensor for temperature and humidity tracking
- **⚙️ Configurable Setpoints**: Remote temperature target setting via ESP8266 commands
- **🎵 Audio Alerts**: Piezo buzzer notifications for over-temperature conditions (+3°C threshold)
- **🔄 Auto/Manual Modes**: Automatic temperature control or manual fan speed override


</details>


<details>
<summary><b>⚡ Power Monitoring & Safety</b></summary>

- **🔌 Electrical Monitoring**: ACS712 current sensor (0.185 V/A) with fixed 220V for power calculations
- **📈 Power Analytics**: Real-time wattage calculation (V×I) and kWh energy tracking
- **🛡️ Safety Features**: Sensor validation, range checking, and NaN detection
- **🔍 RMS Calculation**: True RMS measurement with calibrated sampling (100 samples)
- **📋 Error Handling**: Safe fallback values and error status reporting to ESP8266


</details>


<details>
<summary><b>📡 Inter-ESP Communication</b></summary>

- **� Serial Protocol**: Custom message format `<TYPE:DATA>` at 9600 baud
- **📤 Sensor Data**: Sends temperature, humidity, voltage, current, fan speed every 5 seconds
- **📥 Command Processing**: Receives fan speed and temperature setpoint commands
- **🔄 Health Monitoring**: Connection status and communication timeout handling
- **⏰ Timing Management**: 2-second sensor reads, 5-second data transmission


</details>


### 🌐 ESP8266 WiFi & Firebase Manager


<details>
<summary><b>🌐 Connectivity & Configuration</b></summary>

- **📶 WiFiManager Integration**: User-friendly captive portal setup (192.168.4.1)
- **☁️ Firebase Sync**: Real-time cloud data logging and remote control
- **🔄 Auto-Reconnection**: Robust WiFi connection management
- **🔧 Hardware Reset**: Physical button WiFi reset capability (3-second hold)
- **📡 Access Point Mode**: Automatic fallback for configuration


</details>


<details>
<summary><b>☁️ Cloud Integration & Notifications</b></summary>

- **🔥 Firebase Database**: Real-time device data updates and command handling
- **📱 Push Notifications**: Firebase Cloud Messaging for mobile app alerts
- **📊 Data Logging**: Historical sensor data storage and analytics
- **🎯 Remote Commands**: Processes fan speed and temperature commands from mobile app
- **⏰ Notification Scheduling**: Status updates every 30 seconds, data sync every 5 seconds


</details>


<details>
<summary><b>🏗️ Architecture & Development</b></summary>

- **🧩 Modular Design**: Each component encapsulated in dedicated C++ classes
- **🔧 Maintainable Code**: Clean separation of concerns for easy extension
- **📝 Serial Debugging**: Comprehensive logging for development and troubleshooting
- **⚡ Non-blocking Operations**: Efficient multitasking without interference
- **🎛️ Pin Configuration**: Centralized pin management through `PinConfig.h`


</details>


### 📱 Android App Features


<details>
<summary><b>🔐 User Management & Security</b></summary>

- **🔑 Firebase Authentication**: Secure email/password login and registration
- **🔗 Device Linking**: Secure device association with user accounts
- **🛡️ Access Control**: Firebase security rules for data protection
- **✅ Input Validation**: Comprehensive validation throughout the app
- **👤 Account Management**: Profile management and secure logout


</details>


<details>
<summary><b>📊 Real-time Dashboard</b></summary>

- **🌡️ Animated Temperature Gauge**: Beautiful SpeedView visualization
- **⚡ Power Monitoring Cards**: Live voltage, current, wattage, and energy display
- **🎛️ Fan Control Interface**: Manual speed control and auto/manual mode toggle
- **📈 Status Indicators**: Color-coded power consumption status chips
- **🔔 Live Alerts**: Real-time notifications for temperature and power thresholds


</details>


<details>
<summary><b>🛠️ Device Management & Setup</b></summary>

- **📶 WiFi Configuration**: Scan networks and configure device WiFi through app
- **🔧 Device Setup Wizard**: Step-by-step device linking and configuration
- **🏷️ Device Naming**: Rename and organize multiple devices
- **🔄 Connection Management**: Monitor device connectivity and status
- **📋 Device History**: Access to device configuration and setup logs


</details>


<details>
<summary><b>📈 Analytics & History</b></summary>

- **📊 Historical Data**: View temperature, fan speed, and power consumption logs
- **⏰ Timestamp Tracking**: Detailed historical data with proper time formatting
- **💾 Data Export**: Access to last 50 log entries with filtering capabilities
- **📱 Enhanced Log View**: Custom RecyclerView with structured data display
- **🎯 Conditional Display**: Backward compatibility with older data formats


</details>


<details>
<summary><b>🎨 User Experience & Design</b></summary>

- **🌙 Theme Support**: Light and dark mode with consistent Material Design 3
- **📱 Responsive Design**: Adaptive layouts for different screen sizes
- **✨ Smooth Animations**: Card animations and transition effects
- **🎯 Intuitive Navigation**: User-friendly interface with clear visual hierarchy
- **🔔 Push Notifications**: Firebase Cloud Messaging for real-time alerts


</details>


### 🌐 System Integration


<details>
<summary><b>☁️ Firebase Backend Services</b></summary>

- **🔑 Authentication**: Secure user management with email/password
- **📊 Realtime Database**: Live data synchronization between device and app
- **🔔 Cloud Messaging**: Push notifications for alerts and status updates
- **🛡️ Security Rules**: Role-based access control and data protection
- **📈 Analytics**: Usage tracking and performance monitoring


</details>

---


## ⚡ Quick Start


### 🔧 Hardware Setup


**Required Microcontrollers:**
- 🌐 **ESP8266** (WiFi & Firebase)
- 🧠 **ESP32** (Sensors & Hardware)


**Sensors & Components:**
- 🌡️ **DHT22** (GPIO4, ESP32)
- ⚡ **ACS712** (GPIO34, ESP32)
- 🔊 **Piezo Buzzer** (GPIO25, ESP32)
- 🎛️ **TRIAC Module** (GPIO18, ESP32)
- 🔋 **Power Supplies** (separate 3.3V/5V for each ESP)
- ⚡ **Fixed 220V** (no voltage sensor needed)


**Inter-ESP Communication:**
- ESP8266 D6 (RX) ↔ ESP32 GPIO17 (TX)
- ESP8266 D7 (TX) ↔ ESP32 GPIO16 (RX)
- Common ground


**Additional Hardware:**
- 📡 **Zero Cross Detection** (GPIO5, ESP32)
- 🔘 **WiFi Reset Button** (D3, ESP8266, 3s hold)

### 📊 Power Specifications
- **Voltage Range**: 110V–400V AC
- **Current Capacity**: Up to 8A
- **Protection**: Overload cutoff
- **Module Size**: ~5.7x2.85cm
- **Logic Level**: 3.3V/5V compatible
---

## 🛠️ Installation

### � Hardware Assembly

<details>
<summary><b>📡 Inter-ESP Communication Wiring</b></summary>

**Step 1: Serial Communication Setup**
```
ESP8266 (WiFi/Firebase)          ESP32 (Sensors/Hardware)
┌─────────────────────┐         ┌─────────────────────┐
│                     │         │                     │
│  D7 (TX) ●─────────●───────●● GPIO16 (RX)          │
│  D6 (RX) ●─────────●───────●● GPIO17 (TX)          │
│  GND     ●─────────●───────●● GND                  │
│                     │         │                     │
└─────────────────────┘         └─────────────────────┘
```

**Step 2: Power Connections**
- Each ESP board requires separate 3.3V/5V power supply
- Ensure stable power delivery and common ground reference
- Avoid power supply noise that can affect serial communication

</details>

<details>
<summary><b>🔧 ESP32 Sensor & Component Wiring</b></summary>

```
ESP32                           Sensors & Components
┌─────────────────────┐         
│                     │         DHT22 Temperature/Humidity
│  GPIO4   ●──────────●────────● Data Pin
│  3.3V    ●──────────●────────● VCC
│  GND     ●──────────●────────● GND
│                     │         
│                     │         ACS712 Current Sensor
│  GPIO34  ●──────────●────────● Output (Analog)
│  5V      ●──────────●────────● VCC
│  GND     ●──────────●────────● GND
│                     │         
│                     │         Note: GPIO35 freed up
│                     │         (Voltage sensor removed - using fixed 220V)
│                     │         
│                     │         Piezo Buzzer
│  GPIO25  ●──────────●────────● Positive
│  GND     ●──────────●────────● Negative
│                     │         
│                     │         TRIAC Module (RobotDyn Compatible)
│  GPIO18  ●──────────●────────● PWM Input
│  GPIO5   ●──────────●────────● Zero Cross Detect
│  5V      ●──────────●────────● VCC
│  GND     ●──────────●────────● GND
│                     │         
└─────────────────────┘         
```

**RobotDyn TRIAC Pin Configuration:**
- **TRIAC Output**: GPIO18 (Controls TRIAC gate signal) 
- **Zero Cross Detection**: GPIO5 (Detects AC zero-crossing events)
- Updated from GPIO12/GPIO2 for RobotDyn library compatibility

</details>

<details>
<summary><b>🔧 ESP8266 Connections</b></summary>

```
ESP8266                         Components
┌─────────────────────┐         
│                     │         WiFi Reset Button
│  D3      ●──────────●────────● Button (to GND)
│  3.3V    ●──────────●────────● Pull-up resistor
│                     │         
│                     │         Status LED (Optional)
│  D1      ●──────────●────────● LED Anode
│  GND     ●──────────●────────● LED Cathode (via resistor)
│                     │         
└─────────────────────┘         
```

</details>

<details>
<summary><b>⚡ Power Supply Requirements</b></summary>

**ESP8266:**
- **Input**: 5V USB or 3.3V regulated
- **Current**: ~200mA typical
- **Notes**: Can be powered via micro USB

**ESP32:**
- **Input**: 5V USB or 3.3V regulated  
- **Current**: ~250mA typical
- **Notes**: Can be powered via micro USB

**Sensors:**
- **DHT22**: 3.3V, 2.5mA max
- **ACS712**: 5V, 13mA typical
- **TRIAC Module**: 5V, 20mA max
- **Note**: ZMPT101B voltage sensor removed

**Total Power Budget:**
- **ESP8266 + Communication**: ~200mA @ 5V
- **ESP32 + Sensors**: ~280mA @ 5V (reduced without voltage sensor)
- **Recommended Supply**: 5V @ 1A minimum

**Critical Connections:**
1. **Serial Communication**: Ensure TX→RX and RX→TX crossover
2. **Common Ground**: Both ESPs must share common ground
3. **Power Isolation**: Consider separate power supplies for noise reduction
4. **TRIAC Safety**: Proper isolation for AC fan control

**Pull-up Resistors:**
- DHT22 data line: 4.7kΩ to 3.3V
- I2C lines (if used): 4.7kΩ to 3.3V
- Reset button: 10kΩ to 3.3V

**Capacitors (Recommended):**
- Power supply decoupling: 100µF + 0.1µF near each ESP

</details>

### 💾 Software Setup

<details>
<summary><b>📚 Required Libraries</b></summary>

Install these libraries in Arduino IDE:

**ESP8266 Libraries:**
```bash
- Firebase ESP-Client by Mobizt
- WiFiManager by tzapu (v0.16.0+)
- ArduinoJson by Benoit Blanchon (v6.21.3+)
```

**ESP32 Libraries:**
```bash
- DHT sensor library
- Firebase ESP-Client by Mobizt
- ArduinoJson by Benoit Blanchon (v6.21.3+)
- RBDdimmer (RobotDyn Dimmer Library)
```

**Installation Steps:**
1. Open Arduino IDE
2. Go to **Tools > Manage Libraries**
3. Search for "RBDdimmer" and install the library by RobotDyn
4. Search and install each other library listed above
5. Restart Arduino IDE

**RobotDyn Library Installation:**
- Install through Arduino IDE Library Manager
- Or download from: https://github.com/RobotDynOfficial/RBDDimmer

</details>

<details>
<summary><b>🔥 Firebase Credentials Setup</b></summary>

**ESP8266 Configuration:**

1. **Copy Sample File**:
   ```bash
   # Navigate to ESP8266 project folder
   cd source/esp8266/SmartFan/
   
   # Copy the sample credentials file
   cp firebase_credentials.h.sample firebase_credentials.h
   ```

2. **Edit Firebase Credentials** (`firebase_credentials.h`):
   ```cpp
   // Firebase Web API Key (from Firebase Console > Project Settings > General)
   #define API_KEY "your-web-api-key-here"
   
   // Firebase Realtime Database URL
   #define DATABASE_URL "https://your-project-id-default-rtdb.firebaseio.com/"
   
   // Service Account Authentication
   #define FIREBASE_PROJECT_ID "your-project-id"
   #define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-xxxxx@your-project-id.iam.gserviceaccount.com"
   
   // Private Key (from Service Account JSON file - keep the \n characters!)
   #define PRIVATE_KEY "-----BEGIN PRIVATE KEY-----\n" \
   "your-private-key-content-here\n" \
   "-----END PRIVATE KEY-----\n"
   
   // WiFi Configuration (optional - can be set via WiFiManager)
   #define WIFI_SSID "your-wifi-network-name"
   #define WIFI_PASSWORD "your-wifi-password"
   
   // Device Configuration
   #define DEVICE_ID "SmartFan_ESP8266_001"
   ```

**Getting Firebase Credentials:**

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Select your project (or create a new one)
3. **For Web API Key**: Go to Project Settings > General
4. **For Service Account**: 
   - Go to Project Settings > Service Accounts
   - Click "Generate new private key"
   - Download JSON file and extract required information

</details>

<details>
<summary><b>📱 Android Google Services Setup</b></summary>

1. **Copy Sample File**:
   ```bash
   cd source/android/SmartFan/app/
   cp google-services.json.sample google-services.json
   ```

2. **Replace with Actual File**:
   - Go to Firebase Console > Project Settings > General
   - Add your Android app if not already added
   - Download the `google-services.json` file for your Android app
   - Replace the sample file with the downloaded file

3. **Verify Package Name**:
   - Ensure the package name in `google-services.json` matches `com.qppd.smartfan`
   - Or update the package name in your Firebase project

</details>

<details>
<summary><b>🔒 Security Notes</b></summary>

⚠️ **Important Security Guidelines:**

- **Never commit actual credential files** (`firebase_credentials.h` and `google-services.json`) to version control
- **These files are already included in `.gitignore`** to prevent accidental commits
- **Use environment variables** for production deployments
- **Rotate Firebase keys regularly** for enhanced security
- **Set up Firebase Security Rules** to protect your database

**Firebase Security Rules Example:**
```json
{
  "rules": {
    "users": {
      "$uid": {
        ".read": "$uid === auth.uid",
        ".write": "$uid === auth.uid"
      }
    },
    "devices": {
      "$deviceId": {
        ".read": "auth != null && root.child('devices').child($deviceId).child('owner').val() === auth.uid",
        ".write": "auth != null && root.child('devices').child($deviceId).child('owner').val() === auth.uid"
      }
    }
  }
}
```

</details>

<details>
<summary><b>⚙️ Dual-ESP Firmware Configuration</b></summary>

1. **📥 Clone Repository**:
   ```bash
   git clone https://github.com/qppd/IoT-Smart-Fan.git
   cd IoT-Smart-Fan
   ```

2. **🔧 Configure ESP8266 (WiFi Manager)**:
   - Edit `source/esp8266/SmartFan/firebase_credentials.h` with your Firebase credentials
   - Set your Firebase project credentials
   - Configure default WiFi fallback credentials (optional)

3. **📤 Upload ESP8266 Firmware**:
   - Connect ESP8266 to computer
   - Select "NodeMCU 1.0 (ESP-12E Module)" board in Arduino IDE
   - Upload `source/esp8266/SmartFan/SmartFan.ino`

4. **📤 Upload ESP32 Firmware**:
   - Connect ESP32 to computer  
   - Select "ESP32 Dev Module" board in Arduino IDE
   - Upload `source/esp32/SmartFan/SmartFan.ino`

5. **⚡ Power On & Test**:
   - Power both ESP boards simultaneously
   - Monitor serial outputs for initialization messages
   - Verify communication test passes on both boards

</details>

### 📱 Android App Setup

<details>
<summary><b>🔨 Build Configuration</b></summary>

1. **📁 Open Project**:
   ```bash
   # Open in Android Studio
   source/android/SmartFan/
   ```

2. **🔥 Firebase Setup**:
   - Download `google-services.json` from Firebase Console
   - Place in `app/` directory
   - Ensure package name matches your Firebase project

3. **🏗️ Build & Install**:
   - Build project in Android Studio
   - Install on Android device (API 21+)
   - Grant required permissions

</details>

---

## 📡 WiFi Configuration

### 🌐 First-Time Setup (ESP8266)

<details>
<summary><b>🔧 WiFiManager Configuration Process</b></summary>

1. **📶 Access Point Mode**:
   - ESP8266 creates network: `SmartFan_Setup`
   - No password required (open network)
   - Captive portal automatically opens

2. **🌐 Network Configuration**:
   - Connect device to ESP8266 network
   - Navigate to: `192.168.4.1`
   - Enter WiFi credentials and device settings
   - ESP8266 saves configuration and connects

3. **🔄 Reset Capability**:
   - Hold WiFi reset button (GPIO0) for 3 seconds
   - ESP8266 clears stored credentials
   - Returns to access point mode for reconfiguration

4. **✅ Connection Confirmation**:
   - ESP8266 connects to configured WiFi
   - Firebase connection established
   - ESP32 communication test initiated
   - System ready for operation

</details>

### 📱 Android WiFi Setup

<details>
<summary><b>🛠️ App-Based Configuration</b></summary>

1. **📶 Network Scanning**:
   - App scans for available WiFi networks
   - Automatically detects SmartFan access points
   - Material Design interface for network selection

2. **🔧 Configuration Transfer**:
   - Select target WiFi network
   - Enter credentials and optional device ID
   - App connects to SmartFan AP and transfers config

3. **✅ Completion**:
   - ESP32 receives configuration
   - Automatically connects to home WiFi
   - Ready for device linking in app

</details>

---

## � Troubleshooting & Monitoring


### 🚨 Common Issues

<details>
<summary><b>📡 ESP-to-ESP Communication Problems</b></summary>

**No Communication Between ESP8266 and ESP32:**
1. **Check Wiring**: Verify TX↔RX, RX↔TX connections and common ground
   - ESP8266 D7 (TX) → ESP32 GPIO16 (RX)
   - ESP8266 D6 (RX) → ESP32 GPIO17 (TX)
   - Connect GND pins of both boards
2. **Verify Baud Rate**: Ensure both boards use 9600 baud (**now confirmed stable**)
3. **Power Supply**: Check stable power to both ESP boards
4. **Serial Monitor**: Review error messages in both ESP serial outputs

**Message Timeouts or Corrupted Data:**
- Check for loose connections or electromagnetic interference
- Verify message format compliance: `<TYPE:DATA>` (**now validated in production**)
- Monitor for buffer overflow or memory issues
- Ensure proper message delimiter parsing

**If all above are correct and issues persist, check for firmware updates. As of September 2025, serial comms, DHT, TRIAC, and buzzer are all confirmed working.**

</details>

<details>
<summary><b>📊 Sensor Reading Issues</b></summary>

**ESP32 Sensors Not Reading:**
1. **DHT22**: Verify 3.3V/5V power and data pin (GPIO4) connection
2. **ACS712**: Check analog pin (GPIO34) and ensure correct sensitivity (0.185V/A)
3. **Power Supply**: Ensure stable power to all sensor modules
4. **Wiring**: Check for short circuits or loose connections
5. **Note**: Voltage sensor removed - using fixed 220V value

**Invalid Sensor Values (NaN):**
- DHT22 initialization timeout - check power and data line
- Current sensor returning out-of-range values
- Review sensor validation logic in ESP32 code

</details>

<details>
<summary><b>🌐 WiFi & Firebase Connectivity</b></summary>

**ESP8266 WiFi Connection Failed:**
1. **Captive Portal**: Connect to "SmartFan_Setup" AP and configure at 192.168.4.1
2. **WiFi Reset**: Hold WiFi reset button for 3 seconds to clear stored credentials
3. **Signal Strength**: Ensure strong WiFi signal in installation location
4. **Router Compatibility**: Check 2.4GHz WiFi support (ESP8266 limitation)

**Firebase Not Updating:**
1. **Internet Connectivity**: Verify ESP8266 can reach internet
2. **Firebase Credentials**: Check `firebase_credentials.h` configuration
3. **Authentication**: Verify Firebase security rules and project settings
4. **Database Structure**: Ensure proper Firebase database schema

**NTP Time Sync Issues (ESP8266):**
1. **Problem**: NTP shows success but time is 1970
   - **Cause**: ESP8266 not receiving valid time data from NTP servers
   - **Check Router/Firewall**: Ensure UDP port 123 (NTP) is not blocked
   - **Check ISP Blocking**: Some ISPs block NTP traffic
   - **Try Mobile Hotspot**: Test with phone's mobile data

2. **DNS Server Issues**:
   - Change DNS servers in router to Google DNS (8.8.8.8, 8.8.4.4) or Cloudflare (1.1.1.1, 1.0.0.1)
   - Add manual DNS to ESP8266 code: `WiFi.config(ip, gateway, subnet, dns1, dns2);`

3. **Network Connectivity Tests**:
   ```bash
   # Test NTP accessibility (Windows PowerShell)
   Test-NetConnection -ComputerName pool.ntp.org -Port 123
   
   # Check DNS resolution
   ping pool.ntp.org
   ping time.google.com
   ```

4. **Router Configuration**:
   - Restart router
   - Check if MAC address filtering is enabled
   - Verify router firmware is up to date
   - Ensure NTP is enabled on router (some routers block NTP for security)

**Code Improvements for NTP Issues**:
- Enhanced NTP setup with multiple server testing
- Longer timeout periods for slow connections
- Better error detection and fallback time setting
- Improved diagnostics with DNS verification before NTP attempts

</details>

<details>
<summary><b>🔧 RobotDyn TRIAC Integration Issues</b></summary>

**TRIAC Not Responding:**
1. **Library Installation**: Ensure RBDdimmer library is properly installed
   ```bash
   # Install via Arduino IDE Library Manager
   # Search: "RBDdimmer"
   # Or download: https://github.com/RobotDynOfficial/RBDDimmer
   ```

2. **Pin Configuration Update**: Verify GPIO pins match RobotDyn requirements
   - **TRIAC Output**: GPIO18 (updated from GPIO12)
   - **Zero Cross Detection**: GPIO5 (updated from GPIO2)

3. **Wiring Verification**:
   - Check TRIAC module connections to ESP32
   - Verify zero-cross detection circuit is functioning
   - Ensure proper AC phase detection wiring

4. **Testing and Calibration**:
   ```cpp
   // Uncomment in setup() to test TRIAC functionality
   testTRIACDimmer();
   
   // Tests power levels: 0%, 25%, 50%, 75%, 100%
   // Each level held for 2 seconds
   // Monitor serial output for power readings
   ```

**Common RobotDyn Issues**:
- **No dimming response**: Check zero-cross pin connection (GPIO5)
- **Erratic behavior**: Verify AC phase detection wiring
- **Library errors**: Ensure RBDdimmer library compatibility
- **Power level mismatch**: Library may adjust values internally

**Safety Considerations**:
- Always use proper AC isolation when working with mains voltage
- Test with low-voltage AC sources first
- The TRIAC module should include optical isolation
- Never work on live AC circuits without proper safety equipment

</details>

<details>
<summary><b>⚡ Fan Control & Hardware Issues</b></summary>

**Fan Not Responding:**
1. **TRIAC Wiring**: Check TRIAC module connections and zero-cross detection (GPIO5, updated from GPIO2)
2. **AC Power**: Verify proper AC wiring and safety isolation
3. **Control Signal**: Monitor ESP32 PWM output (GPIO18, updated from GPIO12) with oscilloscope
4. **Load Testing**: Test TRIAC module with resistive load first

**RobotDyn TRIAC Module Issues**:
- **Hardware Requirements**: RobotDyn AC Dimmer Module + ESP32
- **Zero-cross detection**: Usually included with RobotDyn module
- **Pin Compatibility**: ESP32 GPIO18/GPIO5 based on RobotDyn library chart

**Excessive Power Consumption:**
- Check current sensor calibration and scaling factors (ACS712: 0.185V/A)
- Verify TRIAC switching timing and zero-cross synchronization
- Monitor for partial switching or continuous AC conduction
- Note: Using fixed 220V value for power calculations (voltage sensor removed)

</details>

### 📺 Serial Monitor Debugging

**ESP8266 Monitor Output:**
```
=== Smart Fan ESP8266 - Firebase & WiFi Manager ===
WiFi connected: MyNetwork (IP: 192.168.1.105)
Firebase connected successfully
📡 Received from ESP32: <ALL:25.5,65.0,220.0,0.8,75>
🔥 Firebase data sent successfully
📱 Notification sent: Temperature: 25.5°C, Fan: 75%
```

**ESP32 Monitor Output:**
```
=== Smart Fan ESP32 - Sensor & Hardware Controller ===
🌡️ Temperature: 25.5°C, Humidity: 65.0%
⚡ Voltage: 220.0V, Current: 0.800A, Power: 176.00W
🌀 Fan Speed: 75% (Target: 80°C)
📡 Sent to ESP8266: <ALL:25.5,65.0,220.0,0.8,75>
```

### 🔍 Communication Health Monitoring

**Health Check Commands:**
- ESP8266 → ESP32: `<CMD:GET_STATUS>` (every 15 seconds)
- ESP32 → ESP8266: `<STATUS:RUNNING>` (response confirmation)

**Connection Status Indicators:**
- ✅ **Healthy**: Regular data exchange, no timeouts
- ⚠️ **Warning**: Occasional timeouts, retry attempts
- ❌ **Failed**: No communication for >30 seconds

**Monitoring Tips:**
1. **Timing Analysis**: Monitor message intervals and response times
2. **Error Logging**: Track failed message attempts and timeout events
3. **Data Validation**: Verify sensor data ranges and format compliance
4. **Performance Metrics**: Monitor Firebase sync frequency and success rates

---

## �📖 Documentation

### 🏗️ System Architecture

```mermaid
graph TB
    subgraph "ESP32 Hardware Controller"
        A[ESP32] --> B[DHT22 Sensor<br/>GPIO4]
        A --> C[ACS712 Current<br/>GPIO34]
        A --> E[TRIAC Module<br/>GPIO18/GPIO5]
        A --> F[Piezo Buzzer<br/>GPIO25]
        A --> G[Serial Comm<br/>GPIO16/GPIO17]
    end
    
    subgraph "ESP8266 WiFi Manager"
        H[ESP8266] --> I[WiFi Manager]
        H --> J[Firebase Client]
        H --> K[Serial Comm<br/>D6/D7]
        H --> L[WiFi Reset<br/>D3]
    end
    
    subgraph "Cloud Services"
        M[Firebase Auth]
        N[Realtime Database] 
        O[Cloud Messaging]
        P[Security Rules]
    end
    
    subgraph "Mobile App"
        Q[Android App]
        R[Material Design UI]
        S[Device Management]
        T[Real-time Monitoring]
    end
    
    subgraph "Hardware"
        U[AC Fan Motor]
        V[Zero Cross Detection]
        W[TRIAC Control Circuit]
    end
    
    G -.->|9600 baud| K
    E --> V
    E --> W
    W --> U
    J --> M
    J --> N
    J --> O
    J --> P
    Q --> M
    Q --> N
    Q --> O
    
    style A fill:#1976D2,stroke:#fff,stroke-width:2px,color:#fff
    style H fill:#FF5722,stroke:#fff,stroke-width:2px,color:#fff
    style U fill:#4CAF50,stroke:#fff,stroke-width:2px,color:#fff
```

**Key Architecture Updates (September 2025):**
- **Voltage Sensor Removed**: GPIO35 freed up, using fixed 220V value
- **RobotDyn TRIAC Integration**: GPIO18/GPIO5 (updated from GPIO12/GPIO2)
- **Serial Communication Validated**: 9600 baud confirmed stable in production
- **All Core Modules Working**: DHT22, TRIAC, buzzer, and ESP communication operational

### 📊 Database Structure

<details>
<summary><b>🔥 Firebase Realtime Database Schema</b></summary>

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
      },
      "fcmToken": "firebase_messaging_token"
    }
  },
  "devices": {
    "deviceIdABC": {
      "owner": "uid123",
      "name": "Living Room Fan",
      "current": {
        "temperature": 28.5,
        "humidity": 65.0,
        "fanSpeed": 2,
        "mode": "auto",
        "voltage": 220.1,
        "current": 0.150,
        "watt": 33.02,
        "kwh": 0.125,
        "lastUpdate": 1692620000
      },
      "logs": {
        "timestamp_1692620000": {
          "timestamp": 1692620000,
          "temperature": 28.5,
          "fanSpeed": 2,
          "voltage": 220.1,
          "current": 0.150,
          "watt": 33.02,
          "kwh": 0.125
        }
      }
    }
  }
}
```

</details>

### 📁 Project Structure

<details>
<summary><b>📂 Directory Organization</b></summary>

```
SmartFan/
├── 📊 diagram/                     # Circuit diagrams and schematics
│   ├── SmartFan.fzz              # Fritzing project file  
│   └── SmartFan.png              # Circuit diagram image
├── � CONFIG_SETUP.md             # ⚠️ TO BE REMOVED - Merged into README
├── 📄 README.md                   # Main project documentation
└── 💻 source/
    ├── 📄 ESP_COMMUNICATION_GUIDE.md    # ⚠️ TO BE REMOVED - Merged into README
    ├── � ESP_Communication_Test_Guide.txt
    ├── � WIRING_GUIDE.md              # ⚠️ TO BE REMOVED - Merged into README
    ├── �📱 android/SmartFan/            # Android application
    │   ├── 📱 app/
    │   │   ├── 🔑 src/main/java/com/qppd/smartfan/
    │   │   │   ├── auth/               # Authentication activities  
    │   │   │   ├── device/             # Device management & WiFi setup
    │   │   │   ├── MainActivity.java   # Main dashboard activity
    │   │   │   ├── SettingsActivity.java
    │   │   │   ├── HistoryActivity.java
    │   │   │   └── MyFirebaseMessagingService.java
    │   │   ├── 🎨 src/main/res/        # Resources (layouts, drawables, values)
    │   │   ├── 🔥 google-services.json.sample
    │   │   └── 🔥 google-services.json # ⚠️ Add your Firebase config
    │   ├── 🏗️ build.gradle
    │   └── 🏗️ settings.gradle
    ├── 🧠 esp32/SmartFan/              # ESP32 firmware (Hardware Controller)
    │   ├── SmartFan.ino               # Main application logic
    │   ├── 🌡️ DHTSensor.cpp/.h         # DHT22 temperature/humidity sensor
    │   ├── ⚡ CURRENTSensor.cpp/.h      # ACS712 current measurement  
    │   ├── 🎛️ TRIACModule.cpp/.h        # RobotDyn TRIAC control (GPIO18/GPIO5)
    │   ├── 🔊 BUZZERConfig.cpp/.h       # Piezo buzzer alerts
    │   ├── 📡 ESPCommunication.cpp/.h   # Serial communication with ESP8266
    │   ├── � PinConfig.h              # Pin assignments (updated pins)
    │   └── 📄 ROBOTDYN_INTEGRATION.md  # ⚠️ TO BE REMOVED - Merged into README
    └── 🌐 esp8266/SmartFan/            # ESP8266 firmware (WiFi & Firebase)
        ├── SmartFan.ino               # Main application logic
        ├── 🔥 FirebaseConfig.cpp/.h     # Firebase integration & cloud messaging
        ├── 🌐 WiFiManager.cpp/.h        # WiFi management & captive portal
        ├── ⏰ NTPConfig.cpp/.h          # Network time protocol
        ├── 📡 ESPCommunication.cpp/.h   # Serial communication with ESP32
        ├── 📌 PinConfig.h              # Pin assignments
        ├── 🔑 firebase_credentials.h.sample
        ├── 🔑 firebase_credentials.h    # ⚠️ Add your Firebase credentials
        └── 📄 TROUBLESHOOTING_NTP_FIREBASE.md # ⚠️ TO BE REMOVED - Merged into README

Notes:
- ❌ Voltage sensor (ZMPT101B) removed - using fixed 220V value 
- ✅ RobotDyn TRIAC library integration complete
- ✅ All ESP32 GPIO pins updated for RobotDyn compatibility
- ⚠️ Markdown files marked for removal after content merge
```

</details>

---

## 🔧 Advanced Configuration

### ⚙️ ESP32 Firmware Configuration

<details>
<summary><b>�️ Temperature-Based Fan Control</b></summary>

**Automatic Fan Speed Logic:**
Based on temperature difference from target:
```cpp
// Fan control logic in ESP32 SmartFan.ino
if (tempDiff >= 3.0) {
    fanSpeed = 100;  // Maximum speed
} else if (tempDiff >= 2.0) {
    fanSpeed = 80;   // High speed
} else if (tempDiff >= 1.0) {
    fanSpeed = 60;   // Medium speed
} else if (tempDiff >= 0.5) {
    fanSpeed = 40;   // Low speed
} else {
    fanSpeed = 25;   // Minimum speed
}
```

**Temperature Thresholds:**
- **+3°C or more**: 100% fan speed + buzzer alert
- **+2°C to +3°C**: 80% fan speed  
- **+1°C to +2°C**: 60% fan speed
- **+0.5°C to +1°C**: 40% fan speed
- **0°C to +0.5°C**: 25% fan speed
- **Below target**: 10% minimum speed

</details>

<details>
<summary><b>⚡ Power Monitoring Calibration</b></summary>

**Current Sensor (ACS712):**
```cpp
// In CURRENTSensor.cpp
#define ACS_SENSITIVITY 0.185  // 5A module: 185mV/A
#define ACS_OFFSET 2.5         // Zero current voltage
```

**Voltage Configuration:**
```cpp
// Fixed voltage value in ESP32 SmartFan.ino
sensors.voltage = 220.0; // No sensor needed
```

</details>

### 📱 Android App Customization

<details>
<summary><b>🎨 Theme Configuration</b></summary>

**Color Schemes:**
```xml
<!-- In colors.xml -->
<color name="primary">#1976D2</color>
<color name="accent">#FF5722</color>
<color name="temperature_normal">#4CAF50</color>
<color name="temperature_high">#FF9800</color>
<color name="temperature_critical">#F44336</color>
```

**Material Design 3 Components:**
- Dynamic color theming support
- Adaptive layouts for different screen sizes
- Consistent typography and spacing

</details>

---

## 🚨 Alert System

### 📊 Power Consumption Alerts

<details>
<summary><b>⚡ Alert Thresholds</b></summary>

| Status | Power Range | Color | Action |
|--------|-------------|-------|---------|
| 🟢 Low | < 10W | Green | Normal operation |
| 🔵 Normal | 10-50W | Blue | Standard monitoring |
| 🟡 High | 50-100W | Yellow | Caution alert |
| 🔴 Critical | ≥ 100W | Red | Immediate notification |

</details>

### 🌡️ Temperature Monitoring

<details>
<summary><b>🔥 Temperature Alerts</b></summary>

- **📈 Over-temperature**: Buzzer activation at setpoint + 2°C
- **📱 Push Notifications**: Real-time alerts via Firebase FCM
- **⏰ Alert Duration**: 300ms buzzer beep (configurable)
- **🔄 Reset Conditions**: Automatic reset when temperature normalizes

</details>

---

## 🧪 Testing & Validation

### ✅ Hardware Testing

<details>
<summary><b>🔧 Component Validation</b></summary>

**Sensor Testing:**
```bash
# Serial Monitor Output
Temperature: 26.5°C | Humidity: 60%
Voltage: 220.1V | Current: 0.150A
Power: 33.02W | Energy: 0.125kWh
Fan Speed: 65% | Mode: AUTO
```

**TRIAC Testing:**
- Power sweep validation (0-100%)
- Zero-cross detection timing
- Phase angle accuracy verification

</details>

### 📱 App Testing Scenarios

<details>
<summary><b>🧪 Test Cases</b></summary>

1. **✅ Authentication Flow**: Login/register/logout validation
2. **✅ Device Linking**: QR code and manual ID entry testing
3. **✅ Real-time Updates**: Live data synchronization validation
4. **✅ WiFi Configuration**: Captive portal and app-based setup
5. **✅ Power Monitoring**: Threshold alerts and display accuracy
6. **✅ Theme Switching**: Light/dark mode consistency
7. **✅ Offline Handling**: Graceful degradation without connectivity

</details>

---

## 🔒 Security Considerations

### 🛡️ Firebase Security Rules

<details>
<summary><b>🔐 Access Control Rules</b></summary>

```javascript
{
  "rules": {
    "users": {
      "$uid": {
        ".read": "$uid === auth.uid",
        ".write": "$uid === auth.uid"
      }
    },
    "devices": {
      "$deviceId": {
        ".read": "root.child('devices').child($deviceId).child('owner').val() === auth.uid",
        ".write": "root.child('devices').child($deviceId).child('owner').val() === auth.uid"
      }
    }
  }
}
```

</details>

### 🔧 Device Security

<details>
<summary><b>🛡️ Security Features</b></summary>

- **🔐 WPA2 Protection**: SmartFan AP uses secure password
- **⏱️ Timeout Protection**: Configuration portal auto-closes after inactivity
- **🔄 Reset Protection**: Physical button required for WiFi reset
- **📡 Limited Access**: Configuration only available in setup mode

</details>

---

## 🚀 Future Enhancements

### 🔮 Planned Features

<details>
<summary><b>📈 Next Version Roadmap</b></summary>

**🤖 Smart Features:**
- 🧠 Machine learning temperature prediction
- 📊 Advanced analytics and usage patterns
- 🌤️ Weather integration for predictive control
- 📅 Scheduling and automation rules

**📱 App Enhancements:**
- 📈 Interactive charts and data visualization
- 🔔 Customizable notification preferences
- 🏠 Multi-room and multi-device management
- 🎯 Geofencing for automatic control

**🔧 Hardware Improvements:**
- 📶 Bluetooth fallback configuration
- 🔋 Battery backup for settings retention
- 📡 Mesh networking for multiple devices
- 🌡️ Additional sensor types support

</details>

---

## 🤝 Contributing

We welcome contributions from the community! Here's how you can help:

### 🛠️ Development Setup

<details>
<summary><b>🔧 Setup Instructions</b></summary>

1. **📥 Fork and Clone**:
   ```bash
   git clone https://github.com/your-username/IoT-Smart-Fan.git
   cd IoT-Smart-Fan
   ```

2. **🌿 Create Feature Branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **💻 Development Environment**:
   - ESP32: Arduino IDE or PlatformIO
   - Android: Android Studio (latest version)
   - Firebase: Firebase Console access

4. **✅ Testing**:
   - Test hardware changes with actual components
   - Validate app changes on multiple devices
   - Ensure backward compatibility

</details>

### 📋 Contribution Guidelines

- 🐛 **Bug Reports**: Use issue templates with detailed descriptions
- ✨ **Feature Requests**: Discuss new features in issues before implementing
- 📝 **Documentation**: Update README and code comments
- 🧪 **Testing**: Include tests for new functionality
- 🎨 **Code Style**: Follow existing conventions and formatting

---

## 📄 License

<div align="center">

**📱 Android App**: Apache 2.0 License  
**🧠 ESP32 Firmware**: MIT License

See the `LICENSE` file for complete details.

</div>

---

## 👨‍💻 Author

<div align="center">

**Created with ❤️ by [qppd](https://github.com/qppd)**

[![GitHub](https://img.shields.io/badge/GitHub-qppd-black?style=for-the-badge&logo=github)](https://github.com/qppd)
[![Repository](https://img.shields.io/badge/Repository-IoT--Smart--Fan-blue?style=for-the-badge&logo=git)](https://github.com/qppd/IoT-Smart-Fan)

*Transforming everyday appliances into intelligent IoT solutions*

</div>

---

<div align="center">

### 🌟 Star this project if you find it useful!

![Stargazers](https://img.shields.io/github/stars/qppd/IoT-Smart-Fan?style=social)
![Forks](https://img.shields.io/github/forks/qppd/IoT-Smart-Fan?style=social)
![Issues](https://img.shields.io/github/issues/qppd/IoT-Smart-Fan)

**[⬆ Back to Top](#-smartfan-iot-stand-fan-automation--android-app)**

</div>