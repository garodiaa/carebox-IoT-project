# CareBox Hardware Setup Guide

## Overview
This guide will help you set up the CareBox IoT Medicine Reminder System using ESP32, KY-024 Hall Effect sensors, RGB LEDs, and other components.

## Components Required

### Main Components
- **ESP32 Development Board** (1x)
- **KY-024 Hall Effect Sensor Modules** (3x) - for lid detection
- **Common Anode RGB LEDs** (3x) - for compartment indicators
- **DS3231 RTC Module** (1x) - for accurate timekeeping
- **16x2 I2C LCD Display** (1x) - for status display
- **Passive Buzzer** (1x) - for alarm notifications
- **Neodymium Magnets** (3x) - to trigger Hall effect sensors

### Additional Components
- **Resistors**: 220Ω (9x) for RGB LED current limiting
- **Jumper Wires**: Male-to-Male, Male-to-Female
- **Breadboard** or **PCB** for connections
- **Power Supply**: 5V/3.3V compatible
- **Medicine Box** with 3 compartments

## Pin Configuration

### ESP32 Pin Assignments

#### RGB LED Connections (Common Anode)
| Component | Red Pin | Green Pin | Blue Pin | Common Pin |
|-----------|---------|-----------|----------|------------|
| Compartment 1 (Top) | GPIO 13 | GPIO 32 | GPIO 15 | 3.3V |
| Compartment 2 (Middle) | GPIO 12 | GPIO 16 | GPIO 17 | 3.3V |
| Compartment 3 (Bottom) | GPIO 14 | GPIO 18 | GPIO 19 | 3.3V |

#### Hall Effect Sensors (KY-024)
| Compartment | GPIO Pin | KY-024 Connection |
|-------------|----------|-------------------|
| Compartment 1 | GPIO 33 | D0 (Digital Output) |
| Compartment 2 | GPIO 26 | D0 (Digital Output) |
| Compartment 3 | GPIO 27 | D0 (Digital Output) |

#### Other Components
| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| Buzzer | GPIO 4 | Passive buzzer |
| I2C SDA | GPIO 21 | For LCD and RTC |
| I2C SCL | GPIO 22 | For LCD and RTC |

### KY-024 Hall Effect Sensor Wiring
```
KY-024 Module → ESP32
VCC → 3.3V or 5V
GND → GND
D0 → GPIO Pin (33, 26, or 27)
A0 → Not connected (not used)
```

### RGB LED Wiring (Common Anode)
```
RGB LED → ESP32
Common Anode → 3.3V
Red Cathode → GPIO Pin (through 220Ω resistor)
Green Cathode → GPIO Pin (through 220Ω resistor)
Blue Cathode → GPIO Pin (through 220Ω resistor)
```

### I2C Module Connections
```
DS3231 RTC Module → ESP32
VCC → 3.3V
GND → GND
SDA → GPIO 21
SCL → GPIO 22

16x2 I2C LCD → ESP32
VCC → 5V (or 3.3V depending on module)
GND → GND
SDA → GPIO 21
SCL → GPIO 22
```

## Assembly Instructions

### Step 1: Prepare the Medicine Box
1. Choose a medicine box with 3 separate compartments
2. Install neodymium magnets on each compartment lid
3. Position magnets so they align with KY-024 sensors when lids are closed

### Step 2: Mount KY-024 Sensors
1. Mount each KY-024 sensor near the lid area of each compartment
2. Ensure the sensor face is close to where the magnet will be when lid is closed
3. Test the sensor by bringing a magnet close - the blue LED on the module should light up
4. Adjust the sensitivity using the blue potentiometer if needed

### Step 3: Install RGB LEDs
1. Mount one RGB LED per compartment for visual indication
2. Ensure LEDs are visible and clearly associated with each compartment
3. Connect all common anodes to 3.3V through appropriate wiring

### Step 4: Wire the Circuit
1. Follow the pin configuration table above
2. Use a breadboard or custom PCB for clean connections
3. Add 220Ω resistors in series with each RGB LED pin
4. Ensure all ground connections are common

### Step 5: Connect I2C Devices
1. Wire the DS3231 RTC module to I2C pins (21, 22)
2. Wire the 16x2 LCD display to the same I2C bus
3. Verify I2C addresses (RTC: 0x68, LCD: typically 0x27)

### Step 6: Final Assembly
1. Mount the ESP32 in an accessible location
2. Organize wiring for a clean appearance
3. Ensure all connections are secure
4. Test each component individually

## Testing Hardware

### Power-On Test
1. Connect power to ESP32
2. All RGB LEDs should briefly cycle through colors during startup
3. LCD should display "CareBox @MarsLab" and "Hall Sensor Mode"

### Hall Sensor Test
1. Open each compartment lid - blue LED should turn on
2. Close each compartment lid - LED should turn off
3. Serial monitor should show sensor state changes
4. Adjust KY-024 sensitivity if readings are inconsistent

### LED Test
1. Each RGB LED should display colors correctly
2. Colors should be bright and distinct
3. No flickering or dimming issues

### I2C Test
1. LCD should display current time and status
2. RTC should maintain accurate time
3. No "RTC Error" messages should appear

## Troubleshooting

### Hall Sensor Issues
- **Inconsistent readings**: Adjust the blue potentiometer on KY-024
- **No response**: Check wiring and power connections
- **Wrong trigger logic**: Verify magnet polarity and positioning

### LED Issues
- **LEDs not lighting**: Check common anode connection to 3.3V
- **Wrong colors**: Verify RGB pin assignments
- **Dim LEDs**: Check resistor values (220Ω recommended)

### I2C Issues
- **LCD not working**: Verify I2C address using I2C scanner
- **RTC errors**: Check RTC battery and connections
- **Communication errors**: Ensure proper pullup resistors on I2C lines

### Power Issues
- **Random resets**: Ensure adequate power supply (at least 1A)
- **Brown-out detection**: Check power supply stability
- **Component failures**: Verify voltage levels match component requirements

## Safety Considerations

1. **Power Safety**: Use appropriate voltage levels for each component
2. **Magnet Safety**: Keep strong magnets away from electronic devices
3. **Medicine Safety**: Ensure electronics don't interfere with medicine storage
4. **Enclosure**: Consider weatherproof enclosure if needed
5. **Backup Power**: Consider battery backup for RTC module

## Next Steps

After completing hardware setup:
1. Follow the Software Setup Guide to configure the code
2. Set up Blynk IoT platform for remote monitoring
3. Configure WiFi credentials in the code
4. Test the complete system functionality

## Support

For hardware-related issues:
- Check connections using a multimeter
- Use ESP32 serial monitor for debugging
- Verify component specifications match requirements
- Consult component datasheets for detailed information
