# Smart Fan Dual ESP Communication Architecture

## Overview

This Smart Fan system uses a dual-ESP architecture where two ESP microcontrollers work together via serial communication:

- **ESP8266**: WiFi Manager + Firebase Handler
- **ESP32**: Sensor Reader + Hardware Controller

## Hardware Connections

### ESP8266 ↔ ESP32 Serial Communication
- **ESP8266 Side**: D6 (RX) ↔ D7 (TX) (SoftwareSerial)
- **ESP32 Side**: GPIO16 (RX) ↔ GPIO17 (TX) (HardwareSerial2)
- **Baud Rate**: 9600
- **Connection**: ESP8266 TX (D7) → ESP32 RX (GPIO16), ESP8266 RX (D6) → ESP32 TX (GPIO17)
- **Common Ground**: Connect GND pins of both ESP boards

## Responsibilities

### ESP8266 (WiFi & Firebase Module)
- ✅ WiFi connection management
- ✅ Firebase database operations
- ✅ Mobile app notifications
- ✅ Data logging and analytics
- ✅ Remote control commands
- ✅ WiFi reset functionality

### ESP32 (Sensor & Hardware Module) 
- ✅ DHT22 temperature/humidity sensor
- ✅ ACS712 current sensor
- ✅ ZMPT101B voltage sensor
- ✅ TRIAC fan speed control
- ✅ Piezo buzzer alerts
- ✅ Automatic temperature-based fan control
- ✅ Hardware safety monitoring

## Communication Protocol

### Message Format
All messages use the format: `<TYPE:DATA>`
- `<` and `>` are message delimiters
- `TYPE` indicates the message category
- `DATA` contains the actual information

### ESP32 → ESP8266 Messages (Sensor Data)

| Message | Description | Example |
|---------|-------------|---------|
| `<TEMP:25.5>` | Temperature reading (°C) | Temperature: 25.5°C |
| `<HUMID:65.0>` | Humidity reading (%) | Humidity: 65.0% |
| `<VOLT:220.0>` | Voltage reading (V) | Voltage: 220.0V |
| `<CURR:0.8>` | Current reading (A) | Current: 0.8A |
| `<FAN:75>` | Current fan speed (0-100%) | Fan speed: 75% |
| `<BUZZ:ON>` | Buzzer status | Buzzer active |
| `<STATUS:RUNNING>` | System status | System running normally |
| `<ALL:25.5,65.0,220.0,0.8,75>` | Combined sensor data | All readings in one message |

### ESP8266 → ESP32 Messages (Commands)

| Message | Description | Example |
|---------|-------------|---------|
| `<CMD:GET_SENSORS>` | Request all sensor readings | Request fresh data |
| `<CMD:GET_STATUS>` | Request system status | Health check |
| `<SET_FAN:50>` | Set fan speed (0-100%) | Set fan to 50% |
| `<SET_TEMP:28.0>` | Set target temperature (°C) | Target temp: 28°C |
| `<FIREBASE:CONNECTED>` | Firebase connection status | Firebase online |
| `<WIFI:CONNECTED>` | WiFi connection status | WiFi online |
| `<BUZZ:ALERT>` | Trigger buzzer alert | Emergency alert |

## Data Flow

### Normal Operation
1. **ESP32** reads sensors every 2 seconds
2. **ESP32** sends combined sensor data to ESP8266 every 5 seconds
3. **ESP8266** processes data and sends to Firebase every 5 seconds
4. **ESP8266** sends status notifications every 30 seconds
5. **ESP8266** requests fresh data from ESP32 every 3 seconds

### Command Flow
1. **Mobile App/Firebase** → **ESP8266** receives remote commands
2. **ESP8266** → **ESP32** forwards control commands
3. **ESP32** executes hardware control (fan speed, alerts)
4. **ESP32** → **ESP8266** confirms command execution
5. **ESP8266** → **Firebase** logs command execution

## Fan Control Logic

### Automatic Mode (ESP32)
Based on temperature difference from target:
- **+3°C or more**: 100% fan speed
- **+2°C to +3°C**: 80% fan speed  
- **+1°C to +2°C**: 60% fan speed
- **+0.5°C to +1°C**: 40% fan speed
- **0°C to +0.5°C**: 25% fan speed
- **Below target**: 10% minimum speed

### Manual Override (ESP8266 Commands)
- Remote commands can override automatic control
- ESP32 uses the higher of automatic or manual speed
- Commands logged to Firebase for tracking

## Safety Features

### ESP32 Safety
- ✅ Sensor validation (NaN and range checking)
- ✅ TRIAC power limiting (0-100%)
- ✅ Temperature alert buzzer (>target+3°C)
- ✅ Watchdog protection with delays

### ESP8266 Safety  
- ✅ Communication health monitoring
- ✅ ESP32 connection status tracking
- ✅ Firebase connectivity alerts
- ✅ WiFi reset button (3-second hold)

## Error Handling

### Communication Errors
- **Timeout Detection**: 1-second message timeout
- **Connection Health**: Regular ping/pong tests
- **Fallback Mode**: ESP32 continues with last known settings
- **Alerts**: ESP8266 notifies Firebase of connection issues

### Sensor Errors
- **Validation**: Check for NaN and out-of-range values
- **Error Reporting**: ESP32 sends error status to ESP8266
- **Safe Defaults**: Continue operation with safe fallback values
- **Logging**: All errors logged to Firebase

## Setup Instructions

### 1. Hardware Setup
1. Connect ESP8266 D7 to ESP32 GPIO16 (TX to RX)
2. Connect ESP8266 D6 to ESP32 GPIO17 (RX to TX) 
3. Connect GND pins together
4. Connect sensors to ESP32 as per PinConfig.h
5. Ensure both boards have separate power supplies

### 2. Software Setup
1. Upload ESP8266 code to ESP8266 board
2. Upload ESP32 code to ESP32 board
3. Configure WiFi credentials on ESP8266
4. Set up Firebase credentials
5. Power on both boards simultaneously

### 3. Testing
1. Check serial monitors for initialization messages
2. Verify communication test passes on both boards
3. Observe sensor data flow in ESP8266 serial monitor
4. Test Firebase connectivity and data logging
5. Test mobile app notifications

## Monitoring and Debugging

### Serial Monitor Output

**ESP8266 Monitor Shows:**
- WiFi connection status
- Firebase operations
- Received sensor data from ESP32
- Communication health status

**ESP32 Monitor Shows:**
- Sensor readings
- Fan control decisions
- Commands received from ESP8266
- System status updates

### Communication Test
Both boards include test functions that verify:
- Message transmission and reception
- Protocol parsing
- Response timing
- Connection stability

## Troubleshooting

### Common Issues

**No Communication Between ESPs:**
1. Check wiring (TX↔RX, RX↔TX, GND connected)
2. Verify baud rate (9600) on both sides
3. Check power supply stability
4. Review serial monitor for error messages

**ESP32 Sensors Not Reading:**
1. Verify sensor wiring per PinConfig.h
2. Check sensor power supply (3.3V/5V)
3. Test individual sensors separately
4. Review sensor initialization in setup()

**Firebase Not Updating:**
1. Check WiFi connection on ESP8266
2. Verify Firebase credentials
3. Check internet connectivity
4. Review Firebase console for errors

**Fan Not Responding:**
1. Check TRIAC wiring and zero-cross detection
2. Verify power supply to fan circuit
3. Test manual fan control commands
4. Check temperature sensor readings

## Future Enhancements

- [ ] Add OTA (Over-The-Air) updates for both ESPs
- [ ] Implement encrypted communication between ESPs
- [ ] Add more sensor types (air quality, light, etc.)
- [ ] Create web dashboard for monitoring
- [ ] Add machine learning for predictive control
- [ ] Implement energy optimization algorithms

---

*This documentation provides a complete guide for the Smart Fan dual-ESP communication architecture. For technical support, refer to the source code comments and error messages in the serial monitors.*