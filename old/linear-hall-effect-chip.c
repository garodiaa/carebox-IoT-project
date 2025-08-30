#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Declare Wokwi API functions that aren't in wokwi-api.h
extern float pin_voltage(pin_t pin);
extern void pin_write_float(pin_t pin, float voltage);
extern void pin_write_digital(pin_t pin, bool value);
extern void chip_set_display_float(const char* id, float value);
extern void chip_set_virtual_pin(int pin_number, float value);

// Global state pointer
static void *global_chip_state;

typedef struct {
  // Pin references
  pin_t pin_vcc;
  pin_t pin_gnd;
  pin_t pin_d0;  // Digital output
  pin_t pin_a0;  // Analog output
  
  // Internal state
  float sensitivity;     // 0-100 range (represents potentiometer position)
  float magnetic_field;  // -100 to 100 (negative = North pole, positive = South pole)
  float analog_value;    // Current analog value
  bool digital_value;    // Current digital value
  float zero_level;      // No-field reference point (typically around 512 for 10-bit ADC with VCC=3.3V)

  // LED indicators
  bool power_led_on;      // Power LED
  bool threshold_led_on;  // Threshold detection LED
} chip_state_t;

// Called at initialization
void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  
  // Initialize pins
  chip->pin_vcc = pin_init("VCC", INPUT);
  chip->pin_gnd = pin_init("GND", INPUT);
  chip->pin_d0 = pin_init("D0", OUTPUT);
  chip->pin_a0 = pin_init("A0", OUTPUT);
  
  // Set initial state
  chip->sensitivity = 50.0f;      // Default mid-position
  chip->magnetic_field = 0.0f;    // No magnetic field
  chip->zero_level = 512.0f;      // Default midpoint for no field
  chip->power_led_on = false;
  chip->threshold_led_on = false;
  
  // Store pointer in global variable instead of using chip_set_userdata
  global_chip_state = chip;
  
  // Initial output calculation
  chip->analog_value = chip->zero_level;
  chip->digital_value = false;
}

// Calculate analog output value based on magnetic field
float calculate_analog_output(chip_state_t *chip, float voltage) {
  // No power = no output
  if (voltage < 1.0f) {
    return 0.0f;
  }
  
  // Calculate analog output based on magnetic field
  // For 10-bit ADC (0-1023 range):
  // - No field: ~512 (half of max)
  // - South pole (positive): increases toward 1023
  // - North pole (negative): decreases toward 0
  
  // Scale for max field
  float max_deflection = 400.0f; // Max points away from zero_level
  
  // Calculate analog output based on magnetic field strength
  float analog_val = chip->zero_level + (chip->magnetic_field * max_deflection / 100.0f);
  
  // Clamp to valid range
  if (analog_val < 0.0f) analog_val = 0.0f;
  if (analog_val > 1023.0f) analog_val = 1023.0f;
  
  return analog_val;
}

// Calculate digital output value based on analog value and sensitivity
bool calculate_digital_output(chip_state_t *chip, float analog_value) {
  // Calculate threshold based on sensitivity
  // Adjust threshold so that sensitivity has a more noticeable effect
  float threshold_offset = (chip->sensitivity - 50.0f) * 8.0f;
  float threshold = chip->zero_level + threshold_offset;
  
  // For CareBox: We want D0 to be HIGH when magnet is present (lid closed)
  // If analog_value > threshold, we have a strong enough field (south pole)
  // to consider the lid closed
  bool result = analog_value > threshold;
  
  // Debug output to help troubleshoot
  if (result != chip->digital_value) {
    printf("Digital output changed to %s (analog: %.1f, threshold: %.1f)\n", 
           result ? "HIGH (lid CLOSED)" : "LOW (lid OPEN)", 
           analog_value, threshold);
  }
  
  return result;
}

// Process updates when control values change
void chip_on_control_change(const char *id, float value) {
  chip_state_t *chip = (chip_state_t *)global_chip_state;
  
  if (!strcmp(id, "sensitivity")) {
    chip->sensitivity = value;
    printf("Hall sensor sensitivity set to %.1f%%\n", value);
    
    // Immediately update digital output based on new sensitivity
    // This makes the sensitivity change have an immediate effect
    if (chip->power_led_on) {
      chip->digital_value = calculate_digital_output(chip, chip->analog_value);
      printf("Threshold updated: Digital output now %s\n", chip->digital_value ? "HIGH" : "LOW");
    }
  } 
  else if (!strcmp(id, "magnet")) {
    chip->magnetic_field = value;
    printf("Magnetic field set to %.1f (-%d = North, +%d = South)\n", 
           value, (int)value < 0 ? (int)-value : 0, (int)value > 0 ? (int)value : 0);
    
    // For simplicity in the CareBox project:
    // - Strong South field (>50): Digital HIGH (lid closed)
    // - Weak or no field (<50): Digital LOW (lid open)
    if (value >= 50) {
      chip->digital_value = true;  // HIGH when magnet is present (lid closed)
      printf("Strong magnetic field: Lid CLOSED (D0=HIGH)\n");
    } else {
      chip->digital_value = false; // LOW when no magnet (lid open)
      printf("Weak/no magnetic field: Lid OPEN (D0=LOW)\n");
    }
  }
}

// Main simulation loop
void chip_tick(uint32_t time_us) {
  chip_state_t *chip = (chip_state_t *)global_chip_state;
  
  // Get input voltage
  float vcc_voltage = pin_voltage(chip->pin_vcc);
  float gnd_voltage = pin_voltage(chip->pin_gnd);
  float voltage = vcc_voltage - gnd_voltage;
  
  // Check if powered
  if (voltage >= 3.0f) {
    if (!chip->power_led_on) {
      chip->power_led_on = true;
      printf("Hall sensor powered on (%.2fV)\n", voltage);
    }
    
    // Calculate analog output
    chip->analog_value = calculate_analog_output(chip, voltage);
    
    // Calculate digital output
    chip->digital_value = calculate_digital_output(chip, chip->analog_value);
    
    // Update threshold LED state
    if (chip->digital_value != chip->threshold_led_on) {
      chip->threshold_led_on = chip->digital_value;
      printf("Threshold LED %s\n", chip->threshold_led_on ? "ON" : "OFF");
    }
    
    // Update outputs
    pin_write_float(chip->pin_a0, chip->analog_value * voltage / 1023.0f);
    pin_write_digital(chip->pin_d0, chip->digital_value);
    
    // Update displays
    float gauss_value = (chip->analog_value - chip->zero_level) / 2.0f;
    chip_set_display_float("gauss", gauss_value);
  } 
  else {
    // No power
    if (chip->power_led_on) {
      chip->power_led_on = false;
      chip->threshold_led_on = false;
      printf("Hall sensor powered off\n");
    }
    
    // Set outputs to low/ground
    pin_write_digital(chip->pin_a0, false);
    pin_write_digital(chip->pin_d0, false);
    
    // Update displays
    chip_set_display_float("gauss", 0);
  }
  
  // Update virtual pins (for debugging/simulation)
  chip_set_virtual_pin(1001, chip->analog_value);
  chip_set_virtual_pin(1002, chip->digital_value ? 1023 : 0);
}

// Cleanup when simulation ends
void chip_destroy() {
  chip_state_t *chip = (chip_state_t *)global_chip_state;
  free(chip);
  global_chip_state = NULL;
}
