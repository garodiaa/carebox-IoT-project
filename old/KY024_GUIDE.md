# KY-024 Hall Effect Sensor Guide for CareBox

## About the KY-024 Module

The KY-024 Linear Hall Effect sensor module you're using has several important features:

![KY-024 Module](https://www.electronicwings.com/public/images/user_images/images/Arduino/Sensors/Hall_Effect_Sensor/KY-024%20Hall%20effect%20sensor%20module.png)

1. **Pin Configuration**:
   - VCC: Connect to 3.3V or 5V
   - GND: Connect to ground
   - D0: Digital output (HIGH/LOW based on magnetic field)
   - A0: Analog output (varies with magnetic field strength)

2. **Components on Board**:
   - Hall Effect sensor (U18 chip)
   - LM393 comparator 
   - Blue potentiometer for sensitivity adjustment
   - LED indicator (lights up when magnetic field is detected)

## Setup for CareBox Implementation

### Hardware Connection

1. **Wiring**:
   ```
   KY-024 → ESP32
   -----------------
   VCC → 3.3V
   GND → GND
   D0 → GPIO 25/26/27 (for the three compartments)
   A0 → Not used in this implementation
   ```

2. **Sensitivity Adjustment**:
   - The blue potentiometer on the KY-024 module adjusts the threshold for the digital output
   - Turn **clockwise** to **increase** sensitivity (detects weaker magnetic fields)
   - Turn **counterclockwise** to **decrease** sensitivity (requires stronger magnetic fields)
   - Best approach: Place magnet at the desired triggering distance, then adjust until the LED on the module just turns on

### Positioning Tips

1. **Module Orientation**:
   - The sensor is most sensitive to the magnetic field perpendicular to the module's face
   - Orient the KY-024 so the flat side faces the magnet's path

2. **Magnet Polarity**:
   - The KY-024 can detect both North and South poles
   - However, for consistent results, keep the same pole facing the sensor in all compartments
   - South pole typically produces stronger readings

3. **Distance Calibration**:
   - Effective range is typically 10-20mm depending on magnet strength
   - Test with your specific magnets to find the optimal distance

## Testing Your KY-024 Setup

1. **Module Verification**:
   - Power the module without any magnet nearby
   - The onboard LED should be OFF (D0 output HIGH)
   - Bring a magnet close to the sensor
   - The onboard LED should light up (D0 output LOW)

2. **CareBox Integration Test**:
   - Use the `testHallSensors()` function in the `fifth.c` code
   - Close each compartment lid (with magnet) - should read LOW (0)
   - Open each compartment lid - should read HIGH (1)

## Troubleshooting

1. **Inconsistent Readings**:
   - Adjust the potentiometer for better sensitivity
   - Check distance between magnet and sensor
   - Try different magnet sizes/strengths

2. **Sensor Always Triggered/Never Triggered**:
   - If always triggered (LED always ON), turn potentiometer counterclockwise to reduce sensitivity
   - If never triggered (LED never ON), turn potentiometer clockwise to increase sensitivity

3. **Wiring Issues**:
   - Verify all connections are secure
   - Check that you're using the D0 pin (digital output), not the A0 pin
   - Ensure the module is receiving proper power (3.3V is recommended for ESP32)
