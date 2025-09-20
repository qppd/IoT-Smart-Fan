# Smart Fan Dual ESP Wiring Guide

## ESP-to-ESP Communication Wiring

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

## ESP32 Sensor Wiring

```
ESP32                           Sensors
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
│                     │         TRIAC Module (Fan Control)
│  GPIO12  ●──────────●────────● PWM Input
│  GPIO2   ●──────────●────────● Zero Cross Detect
│  5V      ●──────────●────────● VCC
│  GND     ●──────────●────────● GND
│                     │         
└─────────────────────┘         
```

## ESP8266 Connections

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

## Power Supply Recommendations

### ESP8266
- **Input**: 5V USB or 3.3V regulated
- **Current**: ~200mA typical
- **Notes**: Can be powered via micro USB

### ESP32  
- **Input**: 5V USB or 3.3V regulated
- **Current**: ~250mA typical
- **Notes**: Can be powered via micro USB

### Sensors
- **DHT22**: 3.3V, 2.5mA max
- **ACS712**: 5V, 13mA typical
- **TRIAC Module**: 5V, 20mA max
- **Note**: ZMPT101B voltage sensor removed

### Total Power Budget
- **ESP8266 + Communication**: ~200mA @ 5V
- **ESP32 + Sensors**: ~280mA @ 5V (reduced without voltage sensor)
- **Recommended Supply**: 5V @ 1A minimum

## Connection Notes

### Critical Connections
1. **Serial Communication**: Ensure TX→RX and RX→TX crossover
2. **Common Ground**: Both ESPs must share common ground
3. **Power Isolation**: Consider separate power supplies for noise reduction
4. **TRIAC Safety**: Proper isolation for AC fan control

### Pull-up Resistors
- DHT22 data line: 4.7kΩ to 3.3V
- I2C lines (if used): 4.7kΩ to 3.3V
- Reset button: 10kΩ to 3.3V

### Capacitors (Recommended)
- Power supply decoupling: 100µF + 0.1µF near each ESP
- Sensor power: 0.1µF near each sensor VCC

## Safety Warnings

⚠️ **AC Power Safety**
- TRIAC module handles AC mains voltage
- Ensure proper electrical isolation
- Use qualified electrician for AC connections
- Test with low voltage DC fan first

⚠️ **Power Supply**
- Avoid overloading USB ports
- Use external 5V supply for complete system
- Check polarity before connecting
- Add fuses for protection

⚠️ **ESD Protection**
- Handle boards with anti-static precautions
- Avoid hot-plugging while powered
- Use proper grounding when working

## Verification Steps

### 1. Pre-Power Checks
- [ ] Verify all wiring with multimeter
- [ ] Check for shorts between power rails
- [ ] Confirm TX/RX crossover connections
- [ ] Validate sensor VCC voltages

### 2. Power-On Sequence
- [ ] Power ESP8266 first, check WiFi connection
- [ ] Power ESP32 second, verify sensor readings
- [ ] Check serial communication in both directions
- [ ] Test all safety features

### 3. Operational Tests
- [ ] Sensor data flowing to Firebase
- [ ] Fan responding to temperature changes
- [ ] Buzzer alerts working
- [ ] Mobile notifications received

---

*Follow this wiring guide carefully to ensure proper communication between the ESP8266 and ESP32 modules. Double-check all connections before applying power.*