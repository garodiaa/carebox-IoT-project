# CareBox User Guide

## Welcome to CareBox
CareBox is an intelligent IoT-based medicine reminder system that helps you manage medication schedules across three compartments with visual, audio, and mobile notifications.

## Quick Start

### 1. First Time Setup
- Ensure all hardware is connected according to the Hardware Setup Guide
- Power on the device
- LEDs will cycle through colors during startup
- LCD will display "CareBox @MarsLab" then "Hall Sensor Mode"

### 2. Connect to WiFi
- Device will automatically connect using configured credentials
- LCD will show "WiFi Connected" when successful
- If connection fails, check WiFi settings in code

### 3. Connect to Blynk
- Open Blynk Legacy app on your phone
- Ensure project is properly configured
- Device will show "Blynk Connected" on LCD
- You'll see white blinking LEDs briefly to confirm connection

## Using the Blynk App

### Setting Medicine Names
1. Open your Blynk project
2. Use text input widgets V0, V1, V2 to enter medicine names:
   - **V0**: Top compartment (Compartment 1)
   - **V1**: Middle compartment (Compartment 2)  
   - **V2**: Bottom compartment (Compartment 3)
3. Tap "Send" after entering each name

### Setting Alarm Times
1. Use Time Input widgets to set up to 3 alarms per compartment:
   - **Compartment 1**: V3, V4, V5
   - **Compartment 2**: V6, V7, V8
   - **Compartment 3**: V9, V10, V11

2. For each time input:
   - Set the time (hour and minute)
   - Select days of the week
   - Tap "Send" to confirm

3. **LED Confirmation**: Each compartment will show white blinking LED for 3 seconds after setting an alarm

### Monitoring Status
- **V12 & V13**: Display current time and next alarm information
- **Blynk Event Log**: Shows real-time alarm notifications

## Understanding LED Indicators

### LED Color Meanings
| LED State | Meaning | Action Required |
|-----------|---------|-----------------|
| **Off** | Normal state, lid closed | None |
| **Red Blinking** | Medicine alarm active | Take medicine, open lid |
| **Blue Solid** | Lid is open | Close lid when done |
| **Blue Blinking** | Upcoming alarm (30 min warning) | Prepare medicine |
| **White Blinking** | System status (connection/setup) | Wait for completion |
| **Green Solid** | Success confirmation | None |

### LED Locations
- **Top LED**: Compartment 1 (configured as V0)
- **Middle LED**: Compartment 2 (configured as V1)
- **Bottom LED**: Compartment 3 (configured as V2)

## Daily Usage

### Taking Medicine
1. **When alarm activates**:
   - Red blinking LED on compartment
   - Buzzer sounds intermittently
   - Blynk notification sent to phone
   - Terminal shows "ALARM: Medicine name due now"

2. **To acknowledge alarm**:
   - Open the compartment lid
   - Blue LED will turn on
   - Alarm will clear automatically
   - Blynk event notification shows medicine taken

3. **After taking medicine**:
   - Close the compartment lid
   - LED turns off
   - System returns to normal state

### Understanding Notifications

#### Mobile Notifications
- **Blynk Events**: Sent when alarms activate
- **Event History**: View past alarms in Blynk event log
- **Notification Settings**: Configure in Blynk app preferences

#### Audio Alerts
- **Buzzer Pattern**: Intermittent beeping when alarm is active
- **Silence**: Opens when lid is opened or alarm time passes

#### LCD Display
- **Line 1**: Current reminder or next alarm info
- **Line 2**: Current time (HH:MM:SS format)
- **Error States**: Shows "RTC Error" if clock issues occur

## Advanced Features

### Upcoming Alarm Warnings
- System shows blue blinking LED 30 minutes before scheduled time
- Helps prepare medicine in advance
- No sound alert, only visual indication

### Multiple Alarms
- Each compartment supports up to 3 different alarm times
- Alarms can be set for different days of the week
- All alarms operate independently

### Automatic Reset
- Medicine "taken" status resets each minute
- Allows for multiple alarms at same time if needed
- System tracks completion status per alarm

### Time Management
- RTC module maintains accurate time even during power outages
- Time displayed in 24-hour format
- Automatic timezone handling (configure in code if needed)

## Troubleshooting

### Common Issues

#### Alarm Not Triggering
- **Check**: Time settings in Blynk app
- **Check**: Day of week selection
- **Check**: Medicine name is set
- **Check**: RTC time accuracy

#### LED Not Working
- **Red not blinking**: Check if alarm time/day is correct
- **Blue not showing**: Check hall sensor and magnet position
- **No LED at all**: Check power and wiring connections

#### Hall Sensor Issues
- **Inconsistent detection**: Adjust blue potentiometer on KY-024
- **Wrong trigger**: Check magnet position and polarity
- **No response**: Verify wiring and power to sensor

#### Connectivity Issues
- **WiFi problems**: Check network credentials and signal strength
- **Blynk disconnected**: Verify auth token and internet connection
- **Terminal not updating**: Check Blynk app connection

### Error Messages

#### LCD Errors
- **"RTC Error"**: Clock module connection issue
- **"WiFi Failed"**: Network connection problem
- **"Check connection"**: I2C communication error

#### Serial Monitor Messages
- **"[ERROR] Invalid compartment"**: Software error, restart device
- **"RTC Error - Check connection"**: Hardware connection issue
- **"No alarms set"**: No active alarms configured

### Maintenance

#### Regular Checks
- **Weekly**: Verify time accuracy on LCD
- **Monthly**: Test all LEDs and hall sensors
- **As needed**: Clean hall sensors and check magnet attachment

#### Battery Backup
- RTC module has backup battery for timekeeping
- Replace CR2032 battery annually or when time resets frequently

## Tips for Best Experience

### Setting Up Schedules
1. **Start Simple**: Set one alarm per compartment initially
2. **Test First**: Use near-future times to test functionality
3. **Consistent Timing**: Use regular intervals (8AM, 2PM, 8PM)
4. **Day Selection**: Choose appropriate days for each medicine

### Organizing Medicine
1. **Clear Labels**: Label each compartment clearly
2. **Consistent Placement**: Always use same compartment for same medicine
3. **Magnet Position**: Ensure magnets are securely attached to lids
4. **LED Visibility**: Keep LEDs unobstructed for clear indication

### Mobile App Usage
1. **Keep App Open**: Notifications work better with app occasionally opened
2. **Network**: Ensure phone has internet connection for remote monitoring
3. **Event History**: Use Blynk event log to review medication history
4. **Backup Settings**: Save Blynk project QR code for backup

### Troubleshooting Steps
1. **Restart Device**: Power cycle ESP32 if behavior is erratic
2. **Check Connections**: Verify all wires are secure
3. **Serial Monitor**: Use Arduino IDE serial monitor for detailed debugging
4. **Component Testing**: Test individual components if issues persist

## Safety and Important Notes

### Medicine Safety
- **Temperature**: Keep electronics away from medicine storage areas
- **Moisture**: Protect electronics from humidity
- **Interference**: Ensure magnets don't affect stored medications
- **Backup**: Always have manual backup for critical medications

### System Limitations
- **Power Dependency**: Requires constant power for full functionality
- **Network Dependency**: Remote features require internet connection
- **Battery Life**: RTC backup battery needs periodic replacement
- **Range**: Blynk notifications require phone connectivity

### Emergency Procedures
- **Power Outage**: RTC maintains time, but alarms won't sound
- **Network Outage**: Local alarms still work, no remote notifications
- **Device Failure**: Have manual backup schedule available
- **Component Failure**: Individual compartments can fail independently

## Support and Maintenance

### Regular Maintenance Schedule
- **Daily**: Check LCD time display
- **Weekly**: Test alarm functionality
- **Monthly**: Clean sensors and check connections
- **Annually**: Replace RTC battery, update software if needed

### Getting Help
1. **Check this guide** for common solutions
2. **Use serial monitor** for detailed error information
3. **Test components individually** to isolate issues
4. **Consult hardware/software setup guides** for configuration issues

### Upgrading and Modifications
- Keep original working code backed up
- Test modifications thoroughly before relying on them
- Document any changes made for future reference
- Consider impact on medicine safety before making changes

---

**Remember**: This system is designed to assist with medication reminders but should not replace professional medical advice or emergency medical procedures. Always consult healthcare providers for medical questions.
