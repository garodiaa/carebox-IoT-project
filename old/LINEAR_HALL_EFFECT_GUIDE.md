# Linear Hall Effect Sensor Module (KY-024) Integration Guide

## Overview

This document explains how the Linear Hall Effect Sensor Module (KY-024 or equivalent) is integrated into the CareBox project, offering advanced magnetic sensing capabilities compared to simpler approaches in previous versions.

## What is a Linear Hall Effect Sensor Module?

The Linear Hall Effect Sensor Module is a versatile sensor that:
- Detects the presence and polarity (North/South) of magnetic fields
- Provides both analog and digital outputs
- Includes a potentiometer for adjusting the digital output sensitivity
- Can measure the relative strength of magnetic fields

## Hardware Integration

### Connections

The Linear Hall Effect Sensor Modules should be connected to the ESP32 as follows:

| Pin on Module | ESP32 Connection | Description |
|---------------|------------------|-------------|
| VCC (+)       | 3.3V or 5V      | Power supply |
| GND (G)       | GND             | Ground reference |
| D0            | GPIO 25/26/27   | Digital output (HIGH when field exceeds threshold) |
| A0            | GPIO 33/34/35   | Analog output (varies with field strength) |

### Module Placement

1. **Position**: Mount the sensor module where the magnet attached to the compartment lid will pass near it
2. **Orientation**: Align the sensor to reliably detect the magnet's field
3. **Distance**: Keep the maximum distance between sensor and magnet within 10-15mm for reliable detection

## How It Works

### Sensor Characteristics

1. **Analog Output (A0)**:
   - With no magnetic field: ~50% VCC (around 512 on a 10-bit ADC with 3.3V reference)
   - South pole present: Output increases linearly toward VCC (up to ~4.2V)
   - North pole present: Output decreases linearly toward GND (down to ~0.86V)
   - Rate of change: Approximately 2.5mV per Gauss

2. **Digital Output (D0)**:
   - Controlled by on-board potentiometer
   - HIGH when magnetic field exceeds threshold
   - LOW when magnetic field is below threshold
   - Indicated by on-board LED when triggered

### Implementation Logic

The software implementation takes advantage of both outputs:

1. **Digital Output (D0)**: Used for state detection (compartment open/closed)
   - HIGH indicates compartment has been opened 
   - LOW indicates compartment is closed

2. **Analog Output (A0)**: Used for calibration and pole detection
   - Calculates Gauss value based on the difference from baseline
   - Detects polarity of the magnetic field (North vs South)
   - Provides additional diagnostic information

## Software Implementation

The implementation includes these key components:

1. **Calibration Process**:
   ```c
   void calibrateHallSensors() {
     // Takes multiple readings to establish baseline when no magnet is present
     // Stores this as the zero Gauss reference point
   }
   ```

2. **Gauss Calculation**:
   ```c
   float calculateGauss(int compartment, int rawReading) {
     return (rawReading - zeroLevelGauss[compartment]) * GAUSS_PER_STEP;
   }
   ```

3. **State Detection and Response**:
   ```c
   // Uses digital output for state determination
   bool newCompartmentState = (digitalReading == HIGH);
   if (newCompartmentState != compartmentClosed[i]) {
     // Handle state change
   }
   ```

4. **Diagnostic Reporting**:
   ```c
   Serial.printf("[Sensor] Digital: %d, Analog: %d, Gauss: %.2f\n", 
                 digitalReading, analogReading, gaussValue);
   ```

## Adjusting the Module

For optimal performance, adjust the module's potentiometer:

1. **Testing Mode**: Run the `testHallSensors()` function during setup to see real-time sensor readings
2. **Sensitivity Adjustment**: 
   - Turn potentiometer clockwise to increase sensitivity (detects weaker fields)
   - Turn counterclockwise to decrease sensitivity (requires stronger fields)
3. **Verification**: Test with actual compartment opening/closing to confirm reliable triggering

## Advanced Features

The implementation includes several advanced features:

1. **Polarity Detection**: Different LED colors based on magnetic pole
   - GREEN: South pole detected (positive Gauss)
   - RED: North pole detected (negative Gauss)
   - BLUE: No significant field detected

2. **Debouncing**: Prevents false triggers from electrical noise or brief magnetic field fluctuations

3. **Analog Diagnostics**: Continuous monitoring of field strength for troubleshooting

## Troubleshooting

1. **Inconsistent Detection**:
   - Adjust the potentiometer sensitivity
   - Check the distance between sensor and magnet
   - Try reversing the magnet orientation (North/South poles)

2. **No Response**:
   - Verify power connections (the module has a power LED)
   - Ensure the GPIO pins are correctly configured
   - Check the magnet is strong enough

3. **Calibration Issues**:
   - Keep all magnets away during calibration
   - Restart the device if environment changes significantly
   - Consider adjusting the `GAUSS_THRESHOLD` constant in the code

## Comparison to Previous Implementations

| Feature | first.c | second.c | third.c | fourth.c |
|---------|---------|----------|---------|----------|
| Sensor Type | Push Buttons | Reed Switches | Simple Hall Effect | Linear Hall Effect Module |
| Outputs | Digital only | Digital only | Digital only | Digital + Analog |
| Field Detection | None | On/Off | On/Off | Strength + Polarity |
| Adjustability | None | None | None | Potentiometer |
| Diagnostics | Minimal | Basic | Basic | Advanced |

The Linear Hall Effect Module implementation offers the most advanced magnetic sensing capabilities, providing both reliable state detection and additional diagnostic information about the magnetic field's strength and polarity.
