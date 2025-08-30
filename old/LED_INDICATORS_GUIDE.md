# CareBox LED Indicator Guide

## Overview
The CareBox system uses RGB LEDs to provide visual feedback about medication schedules and system status. Each of the three compartments has its own RGB LED that can display different colors and patterns to indicate various states.

## LED States and Their Meanings

### Solid Colors

| Color | State Constant | Meaning |
|-------|---------------|---------|
| **Red Solid** | `LED_RED_SOLID` | Indicates a critical state or error condition that requires immediate attention. |
| **Green Solid** | `LED_GREEN_SOLID` | Confirms successful medication taken. Shows for 3 seconds after a compartment is opened when an alarm is active. |
| **Blue Solid** | `LED_BLUE_SOLID` | Used during testing and initialization. No common usage during normal operation. |
| **White Solid** | `LED_WHITE_SOLID` | Used during testing and initialization. Also briefly shown during startup and when connecting to Blynk. |

### Blinking Patterns

| Color | State Constant | Meaning |
|-------|---------------|---------|
| **Red Blinking** | `LED_RED_BLINK` | **Active Medication Reminder** - Indicates that it's time to take medication from this compartment. Will continue blinking until the compartment is opened. Also triggers the buzzer to sound intermittently. |
| **Blue Blinking** | `LED_BLUE_BLINK` | **Two possible meanings:**<br>1. **Upcoming Medication** - Medication is scheduled within the next 30 minutes.<br>2. **Button Press Acknowledgement** - Brief blue blink for 2 seconds when a compartment is opened but no alarm is active. |
| **White Blinking** | `LED_WHITE_BLINK` | **Alarm Set Confirmation** - Briefly blinks for 3 seconds after an alarm is set or modified through the Blynk app. Also shown briefly when Blynk connects successfully. |

### Special Patterns

| Pattern | Meaning |
|---------|---------|
| **All LEDs Off** | Normal state when no reminders are active or upcoming. |
| **All LEDs White Flash** | System startup complete - final confirmation at the end of initialization. |
| **Sequence of Colors** | During startup, each LED cycles through red, green, blue, and white as a hardware test. |

## System Events and LED Responses

### Medication Reminders
- When it's time to take medication, the corresponding compartment's LED will blink red.
- The buzzer will sound intermittently while any medication reminder is active.
- When you open the compartment, the LED turns solid green for 3 seconds, then turns off.

### Upcoming Medication Indicators
- When medication is scheduled within the next 30 minutes, the corresponding compartment's LED will blink blue.
- This serves as an advance notice of upcoming medication.

### App Interactions
- Setting or modifying an alarm through the Blynk app causes the compartment's LED to blink white for 3 seconds.
- Successful connection to the Blynk service causes all LEDs to blink white briefly.

### Button/Compartment Interactions
- Opening a compartment when no reminder is active causes a brief blue blink for 2 seconds.
- Opening a compartment when a reminder is active causes the LED to turn solid green for 3 seconds.

## LED Function Reference

| Function | Description |
|----------|-------------|
| `setRed(i)` | Sets compartment `i` to solid red |
| `setGreen(i)` | Sets compartment `i` to solid green |
| `setBlue(i)` | Sets compartment `i` to solid blue |
| `setWhite(i)` | Sets compartment `i` to solid white |
| `setRedBlink(i)` | Sets compartment `i` to blinking red |
| `setBlueBlink(i)` | Sets compartment `i` to blinking blue |
| `setWhiteBlink(i)` | Sets compartment `i` to blinking white |
| `turnOffLED(i)` | Turns off the LED for compartment `i` |

## Hardware Notes

- The CareBox uses **common anode RGB LEDs**, which means:
  - LOW signal (0V) turns an LED color ON
  - HIGH signal (3.3V) turns an LED color OFF
- LED updates occur every 100ms for smooth blinking effects.
- Blink interval is defined as 500ms (half a second) for all blinking patterns.
