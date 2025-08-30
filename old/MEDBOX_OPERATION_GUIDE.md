# CareBox Operation with Hall Effect Sensors

## How the CareBox Works

1. **Normal State:**
   - All compartment lids are **closed** with magnets near the Hall Effect sensors
   - Hall sensors read **HIGH** when lid is closed (magnet present)
   - No LEDs are lit

2. **When an Alarm Triggers:**
   - The corresponding compartment's LED starts **blinking RED**
   - The buzzer starts beeping
   - LCD displays which medicine to take

3. **Taking Medicine:**
   - User **opens the lid** of the compartment
   - When lid is opened, magnet moves away from sensor
   - Hall sensor reads **LOW** (no magnetic field)
   - System recognizes medicine has been taken
   - LED turns **solid GREEN** briefly
   - Buzzer stops
   - System records medicine as taken

4. **Closing the Lid:**
   - User puts medicine back and closes the lid
   - Hall sensor reads **HIGH** again (magnet detected)
   - LED blinks **WHITE** briefly to acknowledge
   - LED turns off

## Hall Effect Sensor Logic

- **HIGH (1)** = Magnetic field detected = **Lid CLOSED**
- **LOW (0)** = No magnetic field = **Lid OPEN**

## Testing the System

1. Make sure magnets are properly positioned near each Hall Effect sensor when lids are closed
2. Run the system and verify all sensors read HIGH when lids are closed
3. Open each lid and verify the sensor reads LOW
4. Set an alarm for testing
5. When the alarm triggers, open the corresponding lid to stop it

## Wiring

- Hall Effect Sensor pins (D0) connected to ESP32 pins 25, 26, 27
- Magnets mounted on lids, positioned directly above sensors when closed

## Simulation in Wokwi

When using the custom Linear Hall Effect sensor chip in Wokwi:

1. Set the "Magnet" slider to around 80 to simulate lid closed (HIGH)
2. Set the "Magnet" slider to around 0 to simulate lid open (LOW)

## Troubleshooting

If the system doesn't correctly detect lid opening/closing:

1. Check magnet positioning relative to the sensor
2. Verify proper wiring of Hall Effect sensors
3. Test sensor readings using the testHallSensors() function
4. Adjust sensitivity if using the KY-024 module
