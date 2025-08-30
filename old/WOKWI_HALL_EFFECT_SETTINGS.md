# Wokwi Hall Effect Sensor Simulation Guide

## Optimal Settings for CareBox Simulation

### Key Values to Use

1. **To Simulate LID CLOSED (Magnet Near Sensor)**:
   - Set "Magnet" slider to: **80-100**
   - Expected digital reading: **HIGH**
   - LED status: No LED (or briefly WHITE when just closed)

2. **To Simulate LID OPEN (Magnet Away from Sensor)**:
   - Set "Magnet" slider to: **0-20**
   - Expected digital reading: **LOW**
   - LED status: Briefly GREEN when opened (or RED if alarm active)

3. **Sensitivity Setting**:
   - Default value: **50**
   - Higher values (60-80): More sensitive (detects weaker magnetic fields)
   - Lower values (20-40): Less sensitive (requires stronger magnetic fields)

## Testing Different Scenarios

### 1. Setting Up an Alarm
- All lids closed (Magnet = 80-100)
- Set an alarm for the current time
- Observe the LED starts blinking RED
- Buzzer should activate

### 2. Taking Medicine (Opening Lid)
- Change the "Magnet" slider from 80-100 to 0-20
- Observe the LED turns GREEN
- Buzzer should stop
- Console should show "Compartment X opened"

### 3. Closing Lid After Taking Medicine
- Change the "Magnet" slider from 0-20 back to 80-100
- Observe the LED briefly blinks WHITE
- Console should show "Compartment X closed"

### 4. Testing Sensitivity
If the digital readings are inconsistent:

1. First try different "Magnet" values:
   - For LID CLOSED: Try values between 70-100
   - For LID OPEN: Try values between 0-30

2. If still inconsistent, adjust "Sensitivity":
   - Increase sensitivity (60-80) if it's hard to get a HIGH reading
   - Decrease sensitivity (20-40) if it's hard to get a LOW reading

## What's Happening in the Simulation

The Hall Effect sensor simulation works as follows:

1. The "Magnet" slider represents the magnetic field strength:
   - Positive values (1-100): South pole magnetic field
   - Zero (0): No magnetic field
   - Negative values (-1 to -100): North pole magnetic field

2. The "Sensitivity" slider adjusts the detection threshold:
   - It simulates turning the potentiometer on a physical KY-024 sensor
   - It determines how strong the magnetic field must be to trigger a HIGH reading

3. For the CareBox:
   - We use South pole detection (positive Magnet values)
   - A strong field (Magnet=80-100) means the lid is closed
   - No field (Magnet=0-20) means the lid is open

## Advanced Tips

- If alarms aren't triggering, make sure all compartments show HIGH readings when closed
- If the system doesn't detect lid opening, make sure reading changes from HIGH to LOW
- For debugging, run testHallSensors() to see current readings
- Check the serial monitor for detailed event logs
