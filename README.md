# SmartFan


SmartFan is an IoT-based project for automating and monitoring a standard-size stand fan using an ESP32 microcontroller. It features:
- Automatic fan speed control based on temperature (PID algorithm)
- Real-time temperature, humidity, current, and voltage monitoring
- Buzzer alerts for over-temperature
- Firebase integration for cloud data logging
- Dynamic Wi-Fi configuration using WiFiManager
All hardware abstraction and control logic are modularized for easy maintenance and extension.

---


## Features

- **Automatic Fan Speed Control**: PID-based adjustment of fan speed according to temperature.
- **Temperature & Humidity Monitoring**: DHT22 sensor for environmental data.
- **Current & Voltage Monitoring**: ACS712 and ZMPT101B sensors for power usage and safety.
- **Buzzer Alerts**: Audible warning if temperature exceeds setpoint.
- **Firebase Integration**: Real-time upload of all sensor and control data.
- **Wi-Fi Credentials Management**: User-friendly setup via WiFiManager captive portal.
- **Modular Codebase**: Each hardware component and logic is encapsulated in its own module.

---


## Hardware Requirements

- **ESP32 Microcontroller**
- **DHT22 Temperature and Humidity Sensor**
- **ACS712 Current Sensor** (default: 5A, pin 34)
- **ZMPT101B Voltage Sensor** (pin 35)
- **Piezo Buzzer** (pin 25)
- **TRIAC Module** (for universal motor fans) or **VFD** (for induction motor fans)
- **Relay/Optoisolator** (for TRIAC/VFD control)
- **Power Supply**

---


## Software Requirements

- **Arduino IDE** (latest version)
- **Firebase ESP-Client Library** by Mobizt
- **WiFiManager Library**
- **PID_v1 Library**

---


## Installation

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

---


## How It Works

1. **Wi-Fi Configuration**:
   - On first boot, ESP32 creates an access point (`SmartFan_AP`).
   - Connect and enter Wi-Fi credentials via the captive portal (WiFiManager).

2. **Sensor Readings**:
   - DHT22 measures temperature and humidity.
   - ACS712 measures current; ZMPT101B measures voltage.

3. **Fan Speed Control (PID)**:
   - PID algorithm adjusts fan speed (PWM) based on temperature.
   - Setpoint and PID parameters are configurable in code.

4. **Buzzer Alerts**:
   - If temperature exceeds setpoint + 2°C, buzzer beeps for 300ms.

5. **Data Logging (Firebase)**:
   - All sensor readings and fan control values are sent to Firebase RTDB in real-time.
   - Errors in data upload are printed to serial.

6. **Serial Monitoring**:
   - All values are printed to the serial console for debugging and monitoring.

---


## File Structure

```plaintext
SmartFan/
├── SmartFan.ino              # Main application logic
├── DHTSensor.cpp/.h          # DHT22 temperature/humidity sensor abstraction
├── CURRENTSensor.cpp/.h      # ACS712 current sensor abstraction
├── VOLTAGESensor.cpp/.h      # ZMPT101B voltage sensor abstraction
├── PIDConfig.cpp/.h          # PID control logic
├── BUZZERConfig.cpp/.h       # Buzzer control
├── FirebaseConfig.cpp/.h     # Firebase and Wi-Fi setup
├── firebase_credentials.h    # User credentials for Firebase and Wi-Fi
├── PinConfig.h               # Pin assignments
├── README.md                 # Project documentation
```

---


## Contributing

Contributions are welcome! Please fork the repository and submit a pull request. For major changes, please open an issue first to discuss what you would like to change.

---


## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

---


## Author

Created by [qppd](https://github.com/qppd).
