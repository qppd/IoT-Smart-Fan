# Firebase Streaming Implementation Test Guide

## Overview
This guide outlines how to test the comprehensive Firebase streaming implementation for ESP8266 with real-time manual/auto mode and fan speed control.

## Implementation Features

### ✅ Real-time Firebase Streams
1. **Control Stream** - Monitors `smartfan/devices/{deviceId}/control` for:
   - `mode` changes (manual/auto)
   - `fanSpeed` changes
   - `targetTemperature` changes
   - `manualControl` enable/disable

2. **Non-realtime Token Management** - Periodic checks every 5 minutes for:
   - Device registration tokens for push notifications
   - No streaming overhead - uses direct database reads

### ✅ ESP32 Communication
- Real-time commands sent to ESP32 when Firebase changes detected
- Enhanced ESPCommunication class with `setMode()` method
- Immediate response to database updates

## Testing Procedure

### 1. Setup Test Environment

```bash
# Deploy to ESP8266
# Monitor Serial output at 115200 baud
```

Expected startup logs:
```
=== Smart Fan ESP8266 - Firebase & WiFi Manager ===
Initializing Firebase streams...
Starting control stream for path: smartfan/devices/SmartFan_ESP8266_001/control
Firebase control stream initialized successfully!
Loading tokens from database...
✅ Loaded device_token from {pushId}: {token}
Token loading completed. Total tokens: 1
Firebase streams initialization completed!
Smart Fan ESP8266 Initialized Successfully!
```

### 2. Test Real-time Mode Changes

#### Manual Test:
1. Update Firebase database:
   ```json
   // Path: /smartfan/devices/SmartFan_ESP8266_001/control/mode
   "manual"
   ```

2. Expected ESP8266 serial output:
   ```
   🔥 Control Stream Update Received!
   Path: /mode
   Value: "manual"
   🔄 Mode changed from 'auto' to 'manual'
   📡 Sent mode change to ESP32: manual
   ```

3. Expected ESP32 to receive:
   ```
   <SET_MODE:manual>
   ```

#### Auto Test:
1. Update Firebase database:
   ```json
   // Path: /smartfan/devices/SmartFan_ESP8266_001/control/mode
   "auto"
   ```

2. Expected similar logs with mode change to "auto"

### 3. Test Real-time Fan Speed Changes

1. Update Firebase database:
   ```json
   // Path: /smartfan/devices/SmartFan_ESP8266_001/control/fanSpeed
   75
   ```

2. Expected ESP8266 serial output:
   ```
   🔥 Control Stream Update Received!
   Path: /fanSpeed
   Value: 75
   🌀 Fan speed changed from 50 to 75
   📡 Sent fan speed change to ESP32: 75
   ```

3. Expected ESP32 to receive:
   ```
   <SET_FAN:75>
   ```

### 4. Test Target Temperature Changes

1. Update Firebase database:
   ```json
   // Path: /smartfan/devices/SmartFan_ESP8266_001/control/targetTemperature
   24.5
   ```

2. Expected ESP8266 serial output:
   ```
   🔥 Control Stream Update Received!
   Path: /targetTemperature
   Value: 24.5
   🌡️ Target temperature changed to: 24.5°C
   📡 Sent target temperature to ESP32: 24.5°C
   ```

3. Expected ESP32 to receive:
   ```
   <SET_TEMP:24.5>
   ```

### 5. Test Manual Control Toggle

1. Update Firebase database:
   ```json
   // Path: /smartfan/devices/SmartFan_ESP8266_001/control/manualControl
   true
   ```

2. Expected ESP8266 serial output:
   ```
   🔥 Control Stream Update Received!
   Path: /manualControl
   Value: true
   🎛️ Manual control enabled
   📡 Sent manual control state to ESP32: manual
   ```

### 6. Test Token Management (Non-realtime)

1. Check periodic token loading every 5 minutes:
   ```
   Performing periodic token check...
   Loading tokens from database...
   ✅ Loaded device_token from {pushId}: {token}
   Token loading completed. Total tokens: 1
   ```

### 7. Test Stream Recovery

1. Disconnect internet temporarily
2. Expected recovery logs:
   ```
   ❌ Control Stream Error code: -1, reason: connection timeout
   ⏰ Control stream timed out, attempting to resume...
   ```

3. Reconnect internet
4. Expected stream reinitialization

## Performance Validation

### Response Time Metrics
- **Firebase to ESP8266**: < 2 seconds
- **ESP8266 to ESP32**: < 500ms
- **Total latency**: < 3 seconds end-to-end

### Memory Usage
- Monitor heap fragmentation
- Expected stable operation with >8KB free heap

### Stream Reliability
- Automatic reconnection on network issues
- Graceful handling of stream timeouts
- No memory leaks during long operation

## Troubleshooting

### Common Issues

1. **Stream not initializing**
   - Check Firebase authentication
   - Verify database rules allow read access
   - Ensure NTP time is synchronized

2. **Commands not reaching ESP32**
   - Verify ESP32 communication setup
   - Check baud rate (9600)
   - Test communication with ping/pong

3. **High memory usage**
   - Monitor heap fragmentation
   - Reduce buffer sizes if needed
   - Check for memory leaks

### Debug Commands

```cpp
// Add to loop() for debugging
if (Serial.available()) {
    String cmd = Serial.readString();
    cmd.trim();
    
    if (cmd == "test_mode") {
        espComm.setMode("manual");
    } else if (cmd == "test_speed") {
        espComm.setFanSpeed(80);
    } else if (cmd == "test_temp") {
        espComm.setTargetTemperature(25.0);
    }
}
```

## Expected Database Structure

```json
{
  "smartfan": {
    "devices": {
      "SmartFan_ESP8266_001": {
        "control": {
          "mode": "auto",
          "fanSpeed": 50,
          "targetTemperature": 25.0,
          "manualControl": false
        },
        "current": {
          "temperature": 24.2,
          "fanSpeed": 45,
          "mode": "auto",
          "lastUpdate": 1695456789,
          "voltage": 220.0,
          "current": 0.5,
          "watt": 110.0,
          "kwh": 0.05
        }
      }
    },
    "tokens": {
      "{pushId}": {
        "device_token": "{fcm_token}"
      }
    }
  }
}
```

## Success Criteria

✅ Real-time mode changes propagate within 3 seconds  
✅ Real-time fan speed changes propagate within 3 seconds  
✅ Target temperature changes propagate within 3 seconds  
✅ Manual control toggle works correctly  
✅ Token management works non-realtime (5-minute intervals)  
✅ Stream recovery works after network interruption  
✅ Memory usage remains stable during long operation  
✅ ESP32 communication remains responsive  

## Notes

- Streams are optimized for ESP8266 memory constraints
- Token management is intentionally non-realtime to reduce resource usage
- All control commands provide immediate feedback via serial logs
- Firebase rules must allow read access to `/smartfan/devices/{deviceId}/control`