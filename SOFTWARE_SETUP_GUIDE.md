# CareBox Software Setup Guide

## Overview
This guide covers the software configuration and setup for the CareBox IoT Medicine Reminder System.

## Prerequisites

### Required Software
- **Arduino IDE** (version 1.8.19 or later)
- **ESP32 Board Package** for Arduino IDE
- **Blynk Legacy App** (mobile application)

### Required Libraries
Install these libraries through Arduino IDE Library Manager:

```
1. Blynk (by Volodymyr Shymanskyy) - v1.0.1 or later
2. WiFi (ESP32 built-in)
3. Wire (Arduino built-in)
4. RTClib (by Adafruit) - v2.1.1 or later
5. LiquidCrystal_I2C (by Frank de Brabander) - v1.1.2 or later
```

## Installation Steps

### Step 1: Arduino IDE Setup

1. **Install Arduino IDE**
   - Download from [arduino.cc](https://www.arduino.cc/en/software)
   - Install following the standard procedure for your OS

2. **Add ESP32 Board Support**
   - Open Arduino IDE
   - Go to File → Preferences
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Go to Tools → Board → Board Manager
   - Search for "ESP32" and install "ESP32 by Espressif Systems"

3. **Install Required Libraries**
   - Go to Tools → Manage Libraries
   - Search and install each library listed above
   - Restart Arduino IDE after installation

### Step 2: Blynk Setup

1. **Download Blynk Legacy App**
   - Android: [Google Play Store](https://play.google.com/store/apps/details?id=cc.blynk)
   - iOS: [App Store](https://apps.apple.com/app/blynk-legacy/id808760481)

2. **Create Blynk Account**
   - Open Blynk app
   - Sign up for a new account
   - Verify your email address

3. **Create New Project**
   - Tap "Create New Project"
   - Project Name: "CareBox"
   - Device: "ESP32"
   - Connection: "WiFi"
   - Tap "Create Project"
   - Note down the Auth Token (sent to your email)

### Step 3: Configure Blynk Dashboard

Add these widgets to your Blynk project:

#### Text Input Widgets (Medicine Names)
- **V0**: Medicine Name 1
- **V1**: Medicine Name 2  
- **V2**: Medicine Name 3

#### Time Input Widgets (Alarm Times)
- **V3**: Compartment 1 - Alarm 1
- **V4**: Compartment 1 - Alarm 2
- **V5**: Compartment 1 - Alarm 3
- **V6**: Compartment 2 - Alarm 1
- **V7**: Compartment 2 - Alarm 2
- **V8**: Compartment 2 - Alarm 3
- **V9**: Compartment 3 - Alarm 1
- **V10**: Compartment 3 - Alarm 2
- **V11**: Compartment 3 - Alarm 3

#### Display Widgets
- **V12**: LCD Line 1 (Value Display)
- **V13**: LCD Line 2 (Value Display)

### Step 4: Code Configuration

1. **Open the Code**
   - Open `sketch.c` in Arduino IDE
   - Save as `CareBox.ino` (Arduino sketch format)

2. **Update Credentials**
   ```cpp
   // Replace with your WiFi credentials
   char ssid[] = "YourNetworkName";
   char pass[] = "YourNetworkPassword";
   
   // Replace with your Blynk auth token
   #define BLYNK_AUTH_TOKEN "YourBlynkAuthTokenHere"
   ```

3. **Verify Hardware Pins**
   - Check pin assignments match your wiring
   - Modify if needed for your specific setup

4. **Upload Code**
   - Select Tools → Board → "ESP32 Dev Module"
   - Select correct COM port
   - Click Upload button
   - Monitor serial output for debugging

## Configuration Options

### WiFi Settings
```cpp
char ssid[] = "YourNetworkName";     // Your WiFi network name
char pass[] = "YourNetworkPassword"; // Your WiFi password
```

### Blynk Settings
```cpp
#define BLYNK_TEMPLATE_ID "TMPL6yo8k98jG"      // Keep as is
#define BLYNK_TEMPLATE_NAME "CareBox"          // Keep as is
#define BLYNK_AUTH_TOKEN "YourTokenHere"       // Replace with your token
```

### Hardware Pin Assignments
```cpp
// Modify these if your wiring is different
const int redPins[NUM_COMPARTMENTS]   = {13, 12, 14};
const int greenPins[NUM_COMPARTMENTS] = {32, 16, 18};
const int bluePins[NUM_COMPARTMENTS]  = {15, 17, 19};
const int hallSensorPins[NUM_COMPARTMENTS] = {33, 26, 27};
const int buzzerPin = 4;
```

### Timing Settings
```cpp
#define BLINK_INTERVAL 500        // LED blink speed (ms)
const unsigned long buzzerInterval = 500;  // Buzzer beep interval (ms)
```

## System Behavior

### LED Indicators
- **Red Blinking**: Active alarm (medicine due)
- **Blue Solid**: Lid open (compartment accessed)
- **White Blinking**: System status (connection, alarm set)
- **Off**: Normal state (lid closed, no alarms)

### Hall Effect Sensor Logic
- **HIGH Reading**: Magnet present (lid closed)
- **LOW Reading**: No magnet (lid open)
- **Opening lid**: Clears active alarms, shows blue LED
- **Closing lid**: Turns off LED, resumes normal operation

### Alarm System
- **Active Alarms**: Show red blinking LED + buzzer
- **Medicine Taken**: Opening lid clears alarm
- **Upcoming Alarms**: Blue blinking 30 minutes before due time
- **Multiple Alarms**: Each compartment independent

## Testing and Debugging

### Serial Monitor Output
Enable serial monitoring (115200 baud) to see:
```
[Setup] Starting CareBox with Hall Effect Sensors...
[Setup] Initializing compartment 1: R=13, G=32, B=15, Hall Sensor=33
[WiFi] Connected
[Blynk] Connected, syncing virtual pins...
[Setup] Complete - Hall Effect Sensors ready!
```

### Common Debug Messages
- `[Input] Compartment X opened`: Lid detection working
- `[Alarm] Triggered for Medicine`: Alarm system active
- `[LED] Blue LED on for compartment X`: Visual feedback working
- `[LCD Sync] Updated Blynk`: Blynk communication working

### Testing Sequence
1. **Power On**: LEDs cycle through colors
2. **WiFi Connection**: LCD shows "WiFi Connected"
3. **Blynk Connection**: LCD shows "Blynk Connected"
4. **Hall Sensors**: Open/close lids, observe LED changes
5. **Set Alarms**: Use Blynk app to configure medicine times
6. **Test Alarms**: Set alarm 1-2 minutes in future

## Troubleshooting

### Compilation Errors
- **Library not found**: Install missing libraries
- **Board not selected**: Select ESP32 Dev Module
- **Port issues**: Check USB connection and drivers

### Runtime Issues
- **WiFi not connecting**: Check credentials and network
- **Blynk not connecting**: Verify auth token
- **RTC errors**: Check RTC module connections
- **Hall sensor issues**: Verify wiring and magnet positioning

### Performance Issues
- **Slow response**: Check WiFi signal strength
- **Memory issues**: Monitor available heap memory
- **Timing issues**: Verify RTC time accuracy

## Advanced Configuration

### Custom Time Zones
```cpp
// Add timezone offset in setup() if needed
rtc.adjust(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(0, timeZoneOffset, 0, 0));
```

### Modified Alert Timing
```cpp
// Change upcoming alarm warning time (default: 30 minutes)
if (timeDiff > 0 && timeDiff <= 30) {  // Change 30 to desired minutes
```

### Custom LED Patterns
```cpp
// Modify LED behavior in setPinsColor() function
// Add new LED states in LED constants section
```

## Backup and Updates

### Backing Up Settings
- Export Blynk project QR code
- Save modified Arduino code
- Document any custom pin assignments

### Updating Code
- Always backup current working version
- Test changes with serial monitor active
- Update one feature at a time

### Version Control
- Keep track of code versions
- Document changes made
- Test thoroughly after modifications

## Support and Resources

### Documentation
- [ESP32 Reference](https://docs.espressif.com/projects/esp32/)
- [Blynk Legacy Documentation](https://docs.blynk.io/)
- [Arduino IDE Guide](https://www.arduino.cc/en/Guide)

### Common Resources
- ESP32 pinout diagrams
- KY-024 sensor datasheets
- Blynk widget reference

### Getting Help
- Check serial monitor for error messages
- Verify all connections match the hardware guide
- Test components individually
- Consult component datasheets for specifications
