## CareBox — IoT Medicine Reminder

![Version](https://img.shields.io/badge/version-1.0.0-D32F2F?style=flat)
![ESP32](https://img.shields.io/badge/ESP32-3C3C3C?logo=espressif&logoColor=white&style=flat)
![Arduino](https://img.shields.io/badge/Arduino-00979D?logo=arduino&logoColor=white&style=flat)
![Blynk](https://img.shields.io/badge/Blynk-27AE60?logo=blynk&logoColor=white&style=flat)
![IoT](https://img.shields.io/badge/IoT-FF9800?style=flat)
![License](https://img.shields.io/badge/License-Open%20Source-1565C0?style=flat)

Smart medication reminder that blends ESP32 hardware, hall sensors, RGB indicators, and Blynk notifications to prevent missed doses.

Whether you’re caring for yourself or a loved one, CareBox reduces the risk of missed doses and provides peace of mind with reliable reminders and real-time feedback.

> Struggling to remember your medicine schedule? CareBox is here to help!

**Demo video**: [YouTube walk-through](https://youtu.be/rj_s_OXm1Zs)

### Key Features

**Smart Lid Detection**

- KY-024 hall effect sensors recognize lid openings and closings.
- Magnetic triggers provide consistent activation.
- Alarms clear automatically when a compartment opens.

**Visual Indicators**

- Per-compartment RGB LEDs with color-coded states.
- Real-time feedback for upcoming, active, and acknowledged doses.

**Multi-Channel Notifications**

- Blynk mobile alerts, buzzer audio reminders, 16x2 LCD updates, and serial logs.

**Flexible Scheduling**

- Up to three alarms per compartment (nine total) with weekday targeting, custom labels, and 30-minute warnings.

### System Architecture

**Hardware**

- ESP32 microcontroller
- KY-024 hall sensors (x3)
- Common-anode RGB LEDs (x3)
- DS3231 RTC
- 16x2 I2C LCD
- Passive buzzer
- Neodymium magnets

**Software**

- Arduino IDE + ESP32 core
- Blynk Legacy platform
- RTClib, LiquidCrystal_I2C, and supporting libraries

**Protocols**

- WiFi for IoT communication
- I2C for RTC and LCD
- GPIO for sensors, LEDs, buzzer
- Blynk protocol for cloud/app sync

### Technical Specs

- **Power**: 5V input, 3.3V rails; ~200–500 mA active, <50 mA standby.
- **Sensors**: adjustable hall sensitivity, <10 ms response, -20°C to 85°C operating range.
- **Connectivity**: 2.4 GHz WiFi (802.11 b/g/n), local alarms function offline.

### Project Media

![3D concept](https://github.com/user-attachments/assets/e8d1c69d-8e5f-4781-98b0-3bf0f8215f79)

![Pre-build wiring](https://github.com/user-attachments/assets/872b94dc-2d07-40a0-b470-e98c6b6e3e9e)

![Finished device](https://github.com/user-attachments/assets/4d642ba9-b67d-4084-85a8-46c36b3cf387)

![Internal layout](https://github.com/user-attachments/assets/206e2ada-f629-4c09-ac7e-a73f1aacce0b)

![Mobile UI](https://github.com/user-attachments/assets/f3bdb9ad-0828-4896-a7d5-0c2e5128a83b)

### Repository Layout

```
Final/
├── final.c                   # Arduino sketch
├── HARDWARE_SETUP_GUIDE.md   # Assembly instructions
├── SOFTWARE_SETUP_GUIDE.md   # Library + credential setup
├── USER_GUIDE.md             # Daily-use instructions
└── README.md                 # Project overview
```

### Getting Started

1. **Hardware**: follow HARDWARE_SETUP_GUIDE, check wiring, test components.
2. **Software**: install libs per SOFTWARE_SETUP_GUIDE, set WiFi/Blynk creds.
3. **System Test**: flash `final.c`, confirm boot sequence, verify sensors/LEDs.
4. **User Config**: set medicine names and alarms via USER_GUIDE, run full cycle tests.

**Prerequisites**

- basic electronics and Arduino familiarity
- Blynk app configuration
- comfort with serial debugging

### Operation & Behavior

- Standby → all LEDs off, monitoring RTC.
- Upcoming alarm → blue LED blinks 30 minutes before dose.
- Active alarm → red LED + buzzer until lid opens.
- Acknowledged → solid blue LED while lid remains open.
- Completion → LEDs off on closure, next alarm armed.

**Fault Handling**

- network failures only affect remote notifications
- RTC maintains time across power loss
- compartments operate independently if a sensor fails

### Customization

- expand compartments, swap LEDs/sensors, redesign enclosures.
- tweak timing, LED patterns, notification channels, or Blynk UI.
- integrate with smart homes, health dashboards, databases, or voice assistants.

### Performance & Safety

- debounced sensor reads, automatic WiFi retries, RTC battery backup, watchdog monitoring.
- power-efficient LED logic, memory-conscious firmware, low data usage.
- reminder-only system (not a medical device); keep manual backups and medical oversight.
- low-voltage electronics, isolated from medicine storage, properly grounded and rated.

---

**Status**: Production-ready · **Updated**: August 2025 · **Version**: 1.0 · **License**: Open Source · **Contributors**: IoT Lab Team
