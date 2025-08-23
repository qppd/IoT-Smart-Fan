# SmartFan

SmartFan is an IoT-based project designed to control the speed of a standard-size stand fan based on the temperature sensed by a DHT22 sensor. The project uses an ESP32 microcontroller and integrates Firebase for real-time data monitoring and Wi-Fi credentials management using WiFiManager.

---

## Features

- **Automatic Fan Speed Control**: Adjusts fan speed based on the temperature.
- **Temperature and Humidity Monitoring**: Uses the DHT22 sensor for accurate readings.
- **Firebase Integration**: Real-time data logging and monitoring.
- **Wi-Fi Credentials Management**: Dynamic Wi-Fi configuration using WiFiManager.
- **Compact Design**: Suitable for small home environments.

---

## Hardware Requirements

- **ESP32 Microcontroller**
- **DHT22 Temperature and Humidity Sensor**
- **TRIAC Module (for universal motor fans)** or **VFD (for induction motor fans)**
- **Power Monitoring Module** (e.g., HLW8012 or PZEM-004T)
- **Relay or Optoisolator** (for TRIAC/VFD control)
- **Power Supply**

---

## Software Requirements

- **Arduino IDE** (latest version)
- **Firebase ESP-Client Library** by Mobizt
- **WiFiManager Library**

---

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/qppd/IoT-Smart-Fan.git
   ```

2. **Install Required Libraries**:
   - Open Arduino IDE.
   - Go to **Sketch > Include Library > Manage Libraries...**.
   - Search for and install the following:
     - **Firebase ESP-Client**
     - **WiFiManager**

3. **Configure Firebase**:
   - Replace placeholders in `firebase_credentials.h` with your Firebase project credentials.

4. **Upload the Code**:
   - Connect your ESP32 to your computer.
   - Select the correct board and port in Arduino IDE.
   - Upload the code to the ESP32.

---

## How It Works

1. **Wi-Fi Configuration**:
   - On first boot, the ESP32 creates an access point named `SmartFan_AP`.
   - Connect to the access point and configure Wi-Fi credentials via the portal.

2. **Temperature Sensing**:
   - The DHT22 sensor measures temperature and humidity.

3. **Fan Speed Control**:
   - The fan speed is adjusted based on the temperature using a TRIAC or VFD module.

4. **Firebase Logging**:
   - Temperature and humidity data are sent to Firebase in real-time.

---

## File Structure

```plaintext
SmartFan/
├── DHTSensor.cpp
├── DHTSensor.h
├── FirebaseConfig.cpp
├── FirebaseConfig.h
├── PinConfig.h
├── firebase_credentials.h
├── SmartFan.ino
└── README.md
```

---

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

---

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

---

## Author

Created by [qppd](https://github.com/qppd).
