# Linear Hall Effect Sensor Custom Chip for Wokwi

This guide explains how to use the custom Linear Hall Effect Sensor chip in your Wokwi simulations.

## Files Overview

1. `linear-hall-effect-chip.json` - Defines the chip's interface, pins, and controls
2. `linear-hall-effect-chip.c` - Implements the sensor's behavior

## Setting Up the Custom Chip in Wokwi

1. **Create a New Project** in Wokwi with your ESP32 (or other microcontroller)

2. **Add a Custom Chip**:
   - Click on the "+" button to add a component
   - Search for and select "Custom Chip"

3. **Configure the Custom Chip**:
   - Upload both `linear-hall-effect-chip.json` and `linear-hall-effect-chip.c` files
   - OR copy-paste their contents into the respective editors in Wokwi

4. **Connect the Chip to Your Circuit**:
   - Connect VCC to 3.3V or 5V
   - Connect GND to ground
   - Connect D0 to your digital input pins (pins 25, 26, 27 as per your CareBox design)
   - Connect A0 to your analog input pins if you want to use analog readings (optional)

## Using the Custom Chip

### Pins Description

- **VCC**: Connect to 3.3V or 5V
- **GND**: Connect to ground
- **D0**: Digital output (HIGH when magnetic field exceeds threshold)
- **A0**: Analog output (varies with magnetic field strength)

### Interactive Controls

The chip includes two interactive sliders that appear in the Wokwi interface:

1. **Sensitivity** (0-100):
   - Simulates turning the potentiometer on the physical module
   - Higher values make the sensor more sensitive (easier to trigger)
   - Lower values make the sensor less sensitive (requires stronger magnetic field)

2. **Magnet** (-100 to 100):
   - Simulates a magnet near the sensor
   - Negative values: North pole (-100 = strongest North)
   - Zero: No magnetic field
   - Positive values: South pole (100 = strongest South)

### Visual Feedback

The chip provides visual feedback in the simulation:

- **Gauss Gauge**: Displays the relative magnetic field strength
- **Console Messages**: Information about power state, threshold detection, etc.

## Testing Your CareBox Project

1. **Set Up Multiple Sensors**:
   - Add three custom chips (one for each compartment)
   - Connect them to pins 25, 26, 27 as defined in your code

2. **Simulate Lid Opening/Closing**:
   - To simulate closing a lid: Move the "Magnet" slider to a positive value (e.g., 80)
   - To simulate opening a lid: Move the "Magnet" slider to 0

3. **Fine-Tune Sensitivity**:
   - Adjust the "Sensitivity" slider to find the right trigger point
   - Start at 50 (middle) and adjust as needed

## Implementation Notes

1. **Digital Output Logic**:
   - The digital output (D0) goes HIGH when the magnetic field exceeds the threshold
   - This matches the behavior of the physical KY-024 module

2. **Analog Output Logic**:
   - No field: Output is around half of VCC (512 on 10-bit ADC)
   - South pole: Output increases toward VCC
   - North pole: Output decreases toward GND

3. **Power Requirements**:
   - The chip simulates proper power requirements
   - Outputs only function when VCC is at least 3.0V

## Troubleshooting

If the sensor doesn't seem to be working:

1. **Check Power Connections**: Make sure VCC and GND are properly connected
2. **Verify Pin Connections**: Ensure D0 is connected to the correct input pin
3. **Adjust Sensitivity**: Try increasing the sensitivity if the sensor isn't triggering
4. **Check Magnet Value**: Make sure you're using an appropriate magnetic field strength

## Example Wokwi Diagram

In your `diagram.json`, the connection might look like:

```json
{
  "parts": [
    { "id": "esp", "type": "wokwi-esp32-devkit-v1", "top": 0, "left": 0 },
    { 
      "id": "hallSensor1", 
      "type": "custom-chip", 
      "top": 50, 
      "left": 200, 
      "attrs": {
        "chipId": "linear-hall-effect"
      }
    }
  ],
  "connections": [
    ["esp:3V3", "hallSensor1:VCC"],
    ["esp:GND", "hallSensor1:GND"],
    ["esp:25", "hallSensor1:D0"]
  ]
}
```
