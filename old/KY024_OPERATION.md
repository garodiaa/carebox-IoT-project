# KY-024 Hall Effect Sensor Operation Guide

## How the KY-024 Works with CareBox

![KY-024 Hall Effect Sensor](https://imgaz.staticbg.com/images/oaupload/banggood/images/B6/C8/360765ff-7d48-413b-8dea-399c85ea4e5f.jpg)

### Lid State and Sensor Behavior

| Lid State | Magnet Position | KY-024 Blue LED | Digital Output (D0) | CareBox Action |
|-----------|----------------|-----------------|---------------------|---------------|
| **CLOSED** | Near Sensor | **ON** | **LOW (0)** | Normal operation |
| **OPEN** | Away from Sensor | **OFF** | **HIGH (1)** | Clears alarms, indicates medicine taken |

### Expected Behavior

1. **When Lid is Closed**:
   - The magnet on the lid is close to the KY-024 sensor
   - The blue LED on the KY-024 module will be **ON**
   - The digital output (D0) will read **LOW (0)**
   - Normal operation continues (alarms can be active)

2. **When Lid is Opened**:
   - The magnet moves away from the KY-024 sensor
   - The blue LED on the KY-024 module will turn **OFF**
   - The digital output (D0) will change to **HIGH (1)**
   - System recognizes compartment opening
   - **Any active alarm for that compartment will be cleared**
   - The RGB LED on the CareBox will briefly turn GREEN

### Alarm Behavior

- **Active Alarm + Lid Closed**:
  - Red blinking LED on CareBox
  - Buzzer beeping
  - Blue LED on KY-024 module is ON (magnet detected)

- **Active Alarm + Lid Opened**:
  - Green LED on CareBox (indicates medicine taken)
  - Buzzer stops
  - Blue LED on KY-024 module turns OFF (no magnet detected)
  - Alarm cleared automatically

## Installation Tips

1. **Mount the KY-024 Module**:
   - Position the sensor on the CareBox frame
   - The component side should face where the lid will close

2. **Attach Magnet to Lid**:
   - Secure a small magnet to the underside of each lid
   - Position it to align directly with the KY-024 module when closed
   - Test opening and closing to verify the blue LED on the KY-024 turns on/off

3. **Sensitivity Adjustment**:
   - Use the blue potentiometer on the KY-024 module to adjust sensitivity
   - Turn clockwise to increase sensitivity (detect magnet from further away)
   - Turn counterclockwise to decrease sensitivity (require magnet to be closer)
   - Proper calibration: Blue LED should be ON when lid is closed, OFF when opened

## Testing Your Setup

1. **Basic Functionality Test**:
   - Close all lids and verify blue LEDs on all KY-024 modules are ON
   - Open each lid one by one and verify the blue LED turns OFF when opened
   - The system should print debug messages confirming lid state changes

2. **Alarm Clearing Test**:
   - Set up an alarm for the current time
   - When the alarm activates, the CareBox LED will blink red
   - Open the compartment and verify:
     - Blue LED on KY-024 turns OFF
     - CareBox LED turns green
     - Alarm clears
