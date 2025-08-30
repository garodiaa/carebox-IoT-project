#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6yo8k98jG"
#define BLYNK_TEMPLATE_NAME "CareBox"
#define BLYNK_AUTH_TOKEN "nd2Gd6yMKjrqFs6pEjWPzPVQQYt5miXb"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <climits>

// WiFi credentials - replace with your actual network details
char ssid[] = "DIU_Daffodil Smart City";
char pass[] = "diu123456789";

// KY-024 Hall Effect sensor behavior:
// - HIGH = Magnet present (lid closed) - Blue LED on the KY-024 module will be ON
// - LOW = No magnet (lid open) - Blue LED on the KY-024 module will be OFF

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== LED Control Constants =====
#define LED_OFF        0
#define LED_RED_SOLID  1
#define LED_BLUE_SOLID 2
#define LED_WHITE_SOLID 3
#define LED_GREEN_SOLID 4
#define LED_RED_BLINK  5
#define LED_BLUE_BLINK 6
#define LED_WHITE_BLINK 7

// Blink timing
#define BLINK_INTERVAL 500 // ms

// Using exactly 3 compartments as per the wiring diagram
const int NUM_COMPARTMENTS = 3;
const int ALARMS_PER_COMP = 3;

// LED pin definitions - from the wiring diagram:
// These are COMMON ANODE RGB LEDs (common pin must connect to VCC/3.3V)
// For common anode: drive pins LOW to turn on, HIGH to turn off
// RGB3 = Comp 0: R=13, G=32, B=15 (top)
// RGB2 = Comp 1: R=12, G=16, B=17 (middle)
// RGB1 = Comp 2: R=14, G=18, B=19 (bottom)
const int redPins[NUM_COMPARTMENTS]   = {13, 12, 14};
const int greenPins[NUM_COMPARTMENTS] = {32, 16, 18};
const int bluePins[NUM_COMPARTMENTS]  = {15, 17, 19};

// Hall Effect Sensor pins configuration 
// Digital output (D0) from KY-024 Hall Effect sensor module
// Note: Connect ONLY the D0 pin from the KY-024 modules to these GPIO pins
// The A0 (analog) pin from the KY-024 is not used in this implementation
const int hallSensorPins[NUM_COMPARTMENTS] = {33, 26, 27};
const int buzzerPin = 4;

// Hall Effect sensor states for tracking changes
bool previousHallState[NUM_COMPARTMENTS] = {HIGH, HIGH, HIGH};

// LED state structure
struct LEDControl {
  uint8_t state = LED_OFF;      // LED_OFF, LED_RED_SOLID, etc.
  bool blinkOn = false;       // Whether LED is currently on in blink cycle
  unsigned long lastBlinkTime = 0; // Last toggle timestamp
};

LEDControl ledControls[NUM_COMPARTMENTS];

struct Alarm {
  int hour = -1;
  int minute = -1;
  bool enabled = false;
  bool daysOfWeek[7] = {false};
};

struct MedicineSchedule {
  char medicineName[20] = "Medicine";
  Alarm alarms[ALARMS_PER_COMP];
  bool reminderActive[ALARMS_PER_COMP] = {false};
  bool reminderDone[ALARMS_PER_COMP] = {false};
};

MedicineSchedule schedules[NUM_COMPARTMENTS];
BlynkTimer timer;
DateTime currentTime;
int lastMinute = -1;
bool rtcInitialized = false;
bool buzzerState = false;
unsigned long lastBuzzerTime = 0;
const unsigned long buzzerInterval = 500;
int rtcErrorCount = 0;
const int MAX_RTC_ERRORS = 5;

// ====== Helper Functions ======
// For COMMON ANODE RGB LEDs: 
// - HIGH = LED OFF
// - LOW = LED ON
void setPinsLow(int index) {
  if (index < 0 || index >= NUM_COMPARTMENTS) return;
  
  // Turn off all colors (set pins HIGH for common anode)
  if (redPins[index] >= 0 && redPins[index] < 40) {
    digitalWrite(redPins[index], HIGH);  // HIGH = OFF for common anode
  }
  if (greenPins[index] >= 0 && greenPins[index] < 40) {
    digitalWrite(greenPins[index], HIGH); // HIGH = OFF for common anode
  }
  if (bluePins[index] >= 0 && bluePins[index] < 40) {
    digitalWrite(bluePins[index], HIGH);  // HIGH = OFF for common anode
  }
}

void setPinsColor(int index, uint8_t state) {
  if (index < 0 || index >= NUM_COMPARTMENTS) return;
  
  setPinsLow(index); // Turn all LEDs off first (set pins HIGH)
  
  // Store pin values locally to avoid repeated array access
  int redPin = redPins[index];
  int greenPin = greenPins[index];
  int bluePin = bluePins[index];
  
  // Safety check for all pins
  bool validRedPin = (redPin >= 0 && redPin < 40);
  bool validGreenPin = (greenPin >= 0 && greenPin < 40);
  bool validBluePin = (bluePin >= 0 && bluePin < 40);
  
  // For common anode: drive pins LOW to turn ON the LED
  switch (state) {
    case LED_RED_SOLID:
    case LED_RED_BLINK:
      if (validRedPin) digitalWrite(redPin, LOW);  // LOW = ON for common anode
      break;
    case LED_BLUE_SOLID:
    case LED_BLUE_BLINK:
      if (validBluePin) digitalWrite(bluePin, LOW); // LOW = ON for common anode
      break;
    case LED_WHITE_SOLID:
    case LED_WHITE_BLINK:
      if (validRedPin) digitalWrite(redPin, LOW);   // LOW = ON for common anode
      if (validBluePin) digitalWrite(bluePin, LOW); // LOW = ON for common anode
      if (validGreenPin) digitalWrite(greenPin, LOW); // LOW = ON for common anode
      break;
    case LED_GREEN_SOLID:
      if (validGreenPin) digitalWrite(greenPin, LOW); // LOW = ON for common anode
      break;
  }
}

void setLEDState(int index, uint8_t newState) {
  if (index < 0 || index >= NUM_COMPARTMENTS) return;
  
  if (ledControls[index].state != newState) {
    ledControls[index].state = newState;
    ledControls[index].lastBlinkTime = millis();
    ledControls[index].blinkOn = true;
    setPinsColor(index, newState); // initial ON state
  }
}

// ===== LED Modes =====
void setRed(int i)        { setLEDState(i, LED_RED_SOLID); }
void setBlue(int i)       { setLEDState(i, LED_BLUE_SOLID); }
void setWhite(int i)      { setLEDState(i, LED_WHITE_SOLID); }
void setGreen(int i)      { setLEDState(i, LED_GREEN_SOLID); }
void setRedBlink(int i)   { setLEDState(i, LED_RED_BLINK); }
void setBlueBlink(int i)  { setLEDState(i, LED_BLUE_BLINK); }
void setWhiteBlink(int i) { setLEDState(i, LED_WHITE_BLINK); }
void turnOffLED(int i)    { setLEDState(i, LED_OFF); }

// ===== Main Blink Update =====
void updateLEDs() {
  unsigned long now = millis();

  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    // Safety check - make sure this is a valid compartment
    if (i < 0 || i >= NUM_COMPARTMENTS) {
      continue;  // Skip invalid compartments
    }
    
    uint8_t st = ledControls[i].state;

    if (st == LED_OFF) {
      setPinsLow(i);
      continue;
    }

    bool isBlink = (st == LED_RED_BLINK || st == LED_BLUE_BLINK || st == LED_WHITE_BLINK);

    if (isBlink) {
      if (now - ledControls[i].lastBlinkTime >= BLINK_INTERVAL) {
        ledControls[i].lastBlinkTime = now;
        ledControls[i].blinkOn = !ledControls[i].blinkOn;
        
        // Optional debug for blink state changes (uncomment if needed)
        // Serial.printf("LED %d state %d blink: %s\n", i, st, ledControls[i].blinkOn ? "ON" : "OFF");
      }
      
      if (ledControls[i].blinkOn) {
        setPinsColor(i, st);
      } else {
        setPinsLow(i);
      }
    } else {
      setPinsColor(i, st);
    }
  }
}

bool safeReadRTC() {
  if (!rtcInitialized) return false;
  
  DateTime newTime = rtc.now();
  if (newTime.year() < 2020 || newTime.year() > 2030) {
    rtcErrorCount++;
    if (rtcErrorCount > MAX_RTC_ERRORS) return false;
  } else {
    rtcErrorCount = 0;
    currentTime = newTime;
    return true;
  }
  return false;
}

void processTimeInput(int compIdx, int alarmIdx, const BlynkParam& param) {
  TimeInputParam t(param);
  int h = t.getStartHour();
  int m = t.getStartMinute();
  
  Serial.printf("[Blynk] Comp %d Alarm %d - TimeInput: %02d:%02d\n", compIdx, alarmIdx, h, m);
  
  if (h >= 0 && h <= 23 && m >= 0 && m <= 59) {
    Alarm& a = schedules[compIdx].alarms[alarmIdx];
    a.hour = h;
    a.minute = m;
    a.enabled = true;
    
    // Clear all days first
    for (int d = 0; d < 7; d++) {
      a.daysOfWeek[d] = false;
    }
    
    // Map Blynk weekday selection to RTClib indexing
    // Try both common indexing schemes to ensure compatibility
    // Method 1: Blynk 1-7 (Mon-Sun) to RTClib 0-6 (Sun-Sat)
    for (int blynkDay = 1; blynkDay <= 7; blynkDay++) {
      bool isSelected = t.isWeekdaySelected(blynkDay);
      if (isSelected) {
        int rtcDay = (blynkDay == 7) ? 0 : blynkDay; // Convert: 1=Mon->1, 7=Sun->0
        a.daysOfWeek[rtcDay] = true;
        Serial.printf(" Blynk Day %d -> RTC Day %d: SELECTED\n", blynkDay, rtcDay);
      }
    }
    
    // Also try Method 2: Direct 0-6 mapping as backup
    for (int d = 0; d < 7; d++) {
      if (t.isWeekdaySelected(d)) {
        a.daysOfWeek[d] = true;
        Serial.printf(" Direct mapping Day %d: SELECTED\n", d);
      }
    }
    
    // Brief white blink to indicate alarm set
    setWhiteBlink(compIdx);
    // Auto turn off after 3 seconds
    timer.setTimeout(3000L, [compIdx]() {
      if (ledControls[compIdx].state == LED_WHITE_BLINK) {
        turnOffLED(compIdx);
      }
    });
  }
}

BLYNK_CONNECTED() {
  Serial.println("[Blynk] Connected, syncing virtual pins...");
  Blynk.syncVirtual(V0, V1, V2);
  Blynk.syncVirtual(V3, V4, V5, V6, V7, V8, V9, V10, V11, V12, V13);
  
  // Brief white blink on all LEDs to indicate connection
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    setWhiteBlink(i);
  }
  timer.setTimeout(2000L, []() {
    for (int i = 0; i < NUM_COMPARTMENTS; i++) {
      if (ledControls[i].state == LED_WHITE_BLINK) {
        turnOffLED(i);
      }
    }
  });
}

BLYNK_WRITE(V0) {
  strncpy(schedules[0].medicineName, param.asString(), 19);
  schedules[0].medicineName[19] = '\0';
  Serial.printf("[Blynk] V0 - Medicine Name 1: %s\n", schedules[0].medicineName);
}

BLYNK_WRITE(V1) {
  strncpy(schedules[1].medicineName, param.asString(), 19);
  schedules[1].medicineName[19] = '\0';
  Serial.printf("[Blynk] V1 - Medicine Name 2: %s\n", schedules[1].medicineName);
}

BLYNK_WRITE(V2) {
  strncpy(schedules[2].medicineName, param.asString(), 19);
  schedules[2].medicineName[19] = '\0';
  Serial.printf("[Blynk] V2 - Medicine Name 3: %s\n", schedules[2].medicineName);
}

BLYNK_WRITE(V3) { processTimeInput(0, 0, param); }
BLYNK_WRITE(V4) { processTimeInput(0, 1, param); }
BLYNK_WRITE(V5) { processTimeInput(0, 2, param); }
BLYNK_WRITE(V6) { processTimeInput(1, 0, param); }
BLYNK_WRITE(V7) { processTimeInput(1, 1, param); }
BLYNK_WRITE(V8) { processTimeInput(1, 2, param); }
BLYNK_WRITE(V9) { processTimeInput(2, 0, param); }
BLYNK_WRITE(V10) { processTimeInput(2, 1, param); }
BLYNK_WRITE(V11) { processTimeInput(2, 2, param); }

void checkAlarms() {
  if (!safeReadRTC()) return;
  
  int h = currentTime.hour();
  int m = currentTime.minute();
  int wd = currentTime.dayOfTheWeek();
  
  Serial.printf("[Time] Checking alarms at %02d:%02d (RTC Weekday %d)\n", h, m, wd);
  
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    for (int j = 0; j < ALARMS_PER_COMP; j++) {
      Alarm& a = schedules[i].alarms[j];
      
      if (a.enabled) {
        Serial.printf("[Debug] Alarm %d-%d: %02d:%02d, RTC day %d enabled: %s, active: %s, done: %s\n", 
                      i+1, j+1, a.hour, a.minute, wd, 
                      a.daysOfWeek[wd] ? "YES" : "NO",
                      schedules[i].reminderActive[j] ? "YES" : "NO",
                      schedules[i].reminderDone[j] ? "YES" : "NO");
      }
      
      if (a.enabled && a.hour == h && a.minute == m && 
          a.daysOfWeek[wd] && !schedules[i].reminderActive[j] && 
          !schedules[i].reminderDone[j]) {
        
        schedules[i].reminderActive[j] = true;
        Serial.printf("[Alarm] Triggered for %s (Comp %d, Alarm %d)\n", 
                      schedules[i].medicineName, i + 1, j + 1);
        
        // Start red blinking for this compartment
        setRedBlink(i);
        
        if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
          // Send notification event
          char msg[50];
          snprintf(msg, sizeof(msg), "Time for %s", schedules[i].medicineName);
          Blynk.logEvent("reminder", msg);
        }
      }
    }
  }
}

void handleBuzzer() {
  static unsigned long lastSweepTime = 0;
  static int freq = 1000;
  static int dir = 1; // 1 = up, -1 = down
  const int minFreq = 900;
  const int maxFreq = 2200;
  const int step = 20;
  const unsigned long sweepDelay = 30; // ms
  bool anyActive = false;
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    for (int j = 0; j < ALARMS_PER_COMP; j++) {
      if (schedules[i].reminderActive[j]) {
        anyActive = true;
        break;
      }
    }
    if (anyActive) break;
  }

  if (anyActive) {
    unsigned long now = millis();
    if (now - lastSweepTime > sweepDelay) {
      lastSweepTime = now;
      tone(buzzerPin, freq);
      freq += dir * step;
      if (freq >= maxFreq) {
        freq = maxFreq;
        dir = -1;
      } else if (freq <= minFreq) {
        freq = minFreq;
        dir = 1;
      }
    }
  } else {
    noTone(buzzerPin);
    freq = minFreq;
    dir = 1;
    lastSweepTime = millis();
  }
}

// Check Hall Effect sensors for medicine box compartment lids
void checkHallSensors() {
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    // Read Hall Effect sensor state
    // For KY-024 Hall Effect sensor:
    // - HIGH = Magnet present (lid closed) - Blue LED on the KY-024 module will be ON
    // - LOW = No magnet (lid open) - Blue LED on the KY-024 module will be OFF
    int reading = digitalRead(hallSensorPins[i]);
    
    // Enhanced debounce for KY-024 module
    if (reading != previousHallState[i]) {
      delay(20); // Increased delay for reliable debounce with KY-024
      // Read multiple times to ensure stable reading (KY-024 can be sensitive to noise)
      int reading1 = digitalRead(hallSensorPins[i]);
      delay(5);
      int reading2 = digitalRead(hallSensorPins[i]);
      delay(5);
      int reading3 = digitalRead(hallSensorPins[i]);
      
      // Only accept the change if majority of readings are the same
      if ((reading1 == reading2) || (reading1 == reading3) || (reading2 == reading3)) {
        // Take the majority value
        if (reading1 == reading2) {
          reading = reading1;
        } else if (reading1 == reading3) {
          reading = reading1;
        } else {
          reading = reading2;
        }
        
        // Print debug information about the state change
        Serial.printf("[DEBUG] Hall Sensor %d: Changed from %s to %s\n",
                      i+1,
                      previousHallState[i] == HIGH ? "CLOSED (magnet detected)" : "OPEN (no magnet)",
                      reading == HIGH ? "CLOSED (magnet detected)" : "OPEN (no magnet)");
      } else {
        // Inconsistent readings, keep previous state
        reading = previousHallState[i];
      }
    }
    
    // Detect state change
    if (reading != previousHallState[i]) {
      if (reading == LOW) {
        // LID OPENED (magnet moved away from sensor)
        // LOW reading means no magnetic field detected
        Serial.printf("[Input] Compartment %d opened (Hall Effect triggered)\n", i + 1);
        
        // Show solid BLUE to indicate lid is open
        setBlue(i);
        
        // Force clear ALL active alarms for this compartment
        bool hadActiveReminder = false;
        for (int j = 0; j < ALARMS_PER_COMP; j++) {
          // Always clear reminders when lid is opened, regardless of previous state
          if (schedules[i].reminderActive[j]) {
            schedules[i].reminderActive[j] = false;
            schedules[i].reminderDone[j] = true;
            hadActiveReminder = true;
            Serial.printf("[Alarm] Reminder cleared for %s\n", schedules[i].medicineName);
          }
        }
        
        if (hadActiveReminder) {
          // If there was an active alarm, keep blue LED on while lid is open
          // No timeout - stay blue until lid is closed
          Serial.printf("[Alarm] Active alarm cleared, blue LED on for compartment %d\n", i + 1);
        } else {
          // No active reminder - just show blue LED while lid is open
          // No timeout - stay blue until lid is closed
          Serial.printf("[LED] Blue LED on for compartment %d (lid open)\n", i + 1);
        }
      } else {
        // LID CLOSED (magnet close to sensor)
        // HIGH reading means magnetic field detected
        Serial.printf("[Input] Compartment %d closed\n", i + 1);
        
        // Turn off LED when lid is closed (return to normal state)
        turnOffLED(i);
        Serial.printf("[LED] LED off for compartment %d (lid closed)\n", i + 1);
        
        // Check if there are any pending alarms that should be active
        for (int j = 0; j < ALARMS_PER_COMP; j++) {
          if (schedules[i].reminderActive[j] && !schedules[i].reminderDone[j]) {
            // Restart the alarm indication
            setRedBlink(i);
            Serial.printf("[Alarm] Reactivating alarm for compartment %d\n", i + 1);
            break;
          }
        }
      }
      
      // Update previous state
      previousHallState[i] = reading;
    }
  }
}

// Function to directly test RGB LEDs (for debugging)
void testRGBLEDsDirect() {
  Serial.println("[TEST] Starting quick RGB LED test...");
  
  // Test each LED color directly with reduced delays
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    Serial.printf("[TEST] Testing RGB LED %d\n", i+1);
    
    // Make sure all pins are initialized as outputs
    pinMode(redPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
    pinMode(bluePins[i], OUTPUT);
    
    // Turn all LEDs off first
    digitalWrite(redPins[i], HIGH);   // HIGH = OFF for common anode
    digitalWrite(greenPins[i], HIGH); // HIGH = OFF for common anode
    digitalWrite(bluePins[i], HIGH);  // HIGH = OFF for common anode
    delay(100);
    
    // Test red
    Serial.printf("[TEST] Testing RED pin %d\n", redPins[i]);
    digitalWrite(redPins[i], LOW);    // LOW = ON for common anode
    delay(200);
    digitalWrite(redPins[i], HIGH);   // HIGH = OFF for common anode
    delay(50);
    
    // Test green
    Serial.printf("[TEST] Testing GREEN pin %d\n", greenPins[i]);
    digitalWrite(greenPins[i], LOW);  // LOW = ON for common anode
    delay(200);
    digitalWrite(greenPins[i], HIGH); // HIGH = OFF for common anode
    delay(50);
    
    // Test blue
    Serial.printf("[TEST] Testing BLUE pin %d\n", bluePins[i]);
    digitalWrite(bluePins[i], LOW);   // LOW = ON for common anode
    delay(200);
    digitalWrite(bluePins[i], HIGH);  // HIGH = OFF for common anode
    delay(50);
    
    // Test white (all colors on)
    Serial.println("[TEST] Testing WHITE (all colors)");
    digitalWrite(redPins[i], LOW);    // LOW = ON for common anode
    digitalWrite(greenPins[i], LOW);  // LOW = ON for common anode
    digitalWrite(bluePins[i], LOW);   // LOW = ON for common anode
    delay(200);
    digitalWrite(redPins[i], HIGH);   // HIGH = OFF for common anode
    digitalWrite(greenPins[i], HIGH); // HIGH = OFF for common anode
    digitalWrite(bluePins[i], HIGH);  // HIGH = OFF for common anode
    delay(50);
  }
  
  Serial.println("[TEST] Quick RGB LED test complete");
}

// Function to test Hall Effect sensors (KY-024 modules)
void testHallSensors() {
  Serial.println("[TEST] Testing Hall Effect sensors for CareBox lids...");
  Serial.println("[TEST] KY-024 Hall Effect sensor behavior:");
  Serial.println("[TEST] 1. HIGH reading = Magnet present = LID CLOSED = NO LED on CareBox");
  Serial.println("[TEST] 2. LOW reading = No magnet = LID OPEN = BLUE LED on CareBox");
  Serial.println("[TEST] When lid OPENS: CareBox LED turns BLUE, system clears alarms");
  Serial.println("[TEST] When lid CLOSES: CareBox LED turns OFF, normal operation resumes");
  Serial.println("[TEST] If readings are inconsistent, adjust the blue potentiometer on the KY-024 module.");
  Serial.println("[TEST] Testing current state of all sensors:");
  
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    // White LED during testing
    setWhite(i);
    delay(200);
    
    // Read current state
    int sensorReading = digitalRead(hallSensorPins[i]);
    
    if (sensorReading == HIGH) {
      // Magnet detected - lid is closed - NO LED
      turnOffLED(i);
      Serial.printf("[TEST] Compartment %d: MAGNET DETECTED (HIGH) - LID CLOSED - NO LED\n", i+1);
    } else {
      // No magnet - lid is open - BLUE LED
      setBlue(i);
      Serial.printf("[TEST] Compartment %d: NO MAGNET (LOW) - LID OPEN - BLUE LED\n", i+1);
    }
    
    delay(1000);
    turnOffLED(i);
    delay(200);
  }
  
  Serial.println("[TEST] Hall Effect sensor test complete");
}

void resetDoneFlags() {
  if (!safeReadRTC()) return;
  
  if (currentTime.minute() != lastMinute) {
    for (int i = 0; i < NUM_COMPARTMENTS; i++) {
      for (int j = 0; j < ALARMS_PER_COMP; j++) {
        schedules[i].reminderDone[j] = false;
      }
    }
    
    lastMinute = currentTime.minute();
    Serial.println("[Reset] Reminder done flags reset for new minute.");
  }
}

String getNextAlarmText() {
  if (!safeReadRTC()) return "RTC Error";
  
  int currentHour = currentTime.hour();
  int currentMinute = currentTime.minute();
  int currentWeekday = currentTime.dayOfTheWeek();
  int nextAlarmMinutes = INT_MAX;
  String nextMedicine = "";

  // Check all alarms to find the next one
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    for (int j = 0; j < ALARMS_PER_COMP; j++) {
      Alarm& a = schedules[i].alarms[j];
      if (!a.enabled) continue;

      // Check each day of the week starting from today
      for (int dayOffset = 0; dayOffset < 7; dayOffset++) {
        int checkDay = (currentWeekday + dayOffset) % 7;
        if (!a.daysOfWeek[checkDay]) continue;

        int alarmMinutes;
        if (dayOffset == 0) {
          // Today - only consider future times
          if (a.hour > currentHour || (a.hour == currentHour && a.minute > currentMinute)) {
            alarmMinutes = (a.hour - currentHour) * 60 + (a.minute - currentMinute);
          } else {
            continue; // This alarm already passed today
          }
        } else {
          // Future days
          alarmMinutes = dayOffset * 24 * 60 + (a.hour - currentHour) * 60 + (a.minute - currentMinute);
        }

        if (alarmMinutes < nextAlarmMinutes) {
          nextAlarmMinutes = alarmMinutes;
          nextMedicine = String(schedules[i].medicineName);
        }
      }
    }
  }

  if (nextAlarmMinutes == INT_MAX) {
    return "No alarms set";
  }

  // Format the time remaining
  if (nextAlarmMinutes < 60) {
    return "Next: In " + String(nextAlarmMinutes) + "m";
  } else if (nextAlarmMinutes < 24 * 60) {
    int hours = nextAlarmMinutes / 60;
    int minutes = nextAlarmMinutes % 60;
    return "Next: In " + String(hours) + "h " + String(minutes) + "m";
  } else {
    int days = nextAlarmMinutes / (24 * 60);
    int hours = (nextAlarmMinutes % (24 * 60)) / 60;
    return "Next: In " + String(days) + "d " + String(hours) + "h";
  }
}

void updateLCDs() {
  if (!safeReadRTC()) {
    // Handle RTC error
    lcd.setCursor(0, 0);
    lcd.print("RTC Error       ");
    lcd.setCursor(0, 1);
    lcd.print("Check connection");
    
    if (Blynk.connected()) {
      Blynk.virtualWrite(V12, "RTC Error");
      Blynk.virtualWrite(V13, "Check connection");
    }
    Serial.println("[LCD Sync] RTC Error displayed");
    return;
  }

  bool reminderFound = false;
  String reminderText = "";
  for (int i = 0; i < NUM_COMPARTMENTS && !reminderFound; i++) {
    for (int j = 0; j < ALARMS_PER_COMP && !reminderFound; j++) {
      if (schedules[i].reminderActive[j]) {
        reminderText = "Take: " + String(schedules[i].medicineName);
        reminderFound = true;
      }
    }
  }
  
  // If no active reminder, find next upcoming alarm
  if (!reminderFound) {
    reminderText = getNextAlarmText();
  }

  char timeStr[20];
  snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d:%02d", 
           currentTime.hour(), currentTime.minute(), currentTime.second());

  // Update physical LCD with padding (always updated for real-time display)
  lcd.setCursor(0, 0);
  lcd.print(reminderText);
  lcd.print("                ");  // Simple padding approach from old sketch
  lcd.setCursor(0, 1);
  lcd.print(timeStr);
  lcd.print("                ");  // Simple padding approach from old sketch

  // OPTIMIZATION: Only update Blynk when content actually changes
  static String lastReminderText = "";
  static int lastHour = -1;
  static int lastMinute = -1;
  
  if (Blynk.connected() && 
      (reminderText != lastReminderText || 
       currentTime.hour() != lastHour || 
       currentTime.minute() != lastMinute)) {
    
    // Only send time without seconds to reduce update frequency
    char blynkTimeStr[20];
    snprintf(blynkTimeStr, sizeof(blynkTimeStr), "Time: %02d:%02d", 
             currentTime.hour(), currentTime.minute());
    
    Blynk.virtualWrite(V12, reminderText);  // First line
    Blynk.virtualWrite(V13, blynkTimeStr);  // Second line (no seconds)
    
    lastReminderText = reminderText;
    lastHour = currentTime.hour();
    lastMinute = currentTime.minute();
    
    Serial.printf("[LCD Sync] Updated Blynk: %s | %s\n", reminderText.c_str(), blynkTimeStr);
  }

  Serial.printf("[LCD Sync] %s | %s\n", reminderText.c_str(), timeStr);
}

void indicateUpcomingAlarms() {
  if (!safeReadRTC()) return;
  
  int currentHour = currentTime.hour();
  int currentMinute = currentTime.minute();
  int currentWeekday = currentTime.dayOfTheWeek();
  
  // Check for alarms in the next 30 minutes
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    bool hasUpcoming = false;
    bool hasActive = false;
    
    // Safety check - make sure this is a valid compartment
    if (i < 0 || i >= NUM_COMPARTMENTS) {
      Serial.printf("[ERROR] Invalid compartment index: %d\n", i);
      continue;
    }
    
    // First check if there's an active reminder
    for (int j = 0; j < ALARMS_PER_COMP; j++) {
      if (schedules[i].reminderActive[j]) {
        hasActive = true;
        break;
      }
    }
    
    if (!hasActive && ledControls[i].state == LED_OFF) {
      // Check for upcoming alarms
      for (int j = 0; j < ALARMS_PER_COMP; j++) {
        Alarm& a = schedules[i].alarms[j];
        if (!a.enabled || !a.daysOfWeek[currentWeekday]) continue;
        
        int alarmTotalMinutes = a.hour * 60 + a.minute;
        int currentTotalMinutes = currentHour * 60 + currentMinute;
        int timeDiff = alarmTotalMinutes - currentTotalMinutes;
        
        // If alarm is within next 30 minutes
        if (timeDiff > 0 && timeDiff <= 30) {
          hasUpcoming = true;
          Serial.printf("[Alarm] Upcoming alarm for %s in %d minutes\n", 
                        schedules[i].medicineName, timeDiff);
          break;
        }
      }
      
      if (hasUpcoming) {
        Serial.printf("[LED] Setting blue blink for compartment %d (upcoming alarm)\n", i+1);
        setBlueBlink(i);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("[Setup] Starting CareBox with Hall Effect Sensors...");

  // Initialize RGB LED pins
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    // Debug output to help identify pin configuration issues
    Serial.printf("[Setup] Initializing compartment %d: R=%d, G=%d, B=%d, Hall Sensor=%d\n",
                  i+1, redPins[i], greenPins[i], bluePins[i], hallSensorPins[i]);
    
    // Set all RGB pins as OUTPUT
    pinMode(redPins[i], OUTPUT);
    pinMode(greenPins[i], OUTPUT);
    pinMode(bluePins[i], OUTPUT);
    
    // Ensure all LEDs start in the OFF state
    // For common anode LEDs: HIGH = OFF
    digitalWrite(redPins[i], HIGH);
    digitalWrite(greenPins[i], HIGH);
    digitalWrite(bluePins[i], HIGH);
    
    // Configure Hall Effect sensor pins
    // For physical KY-024 Hall Effect sensors:
    // - HIGH (1) when magnet is present (lid closed) - Blue LED on the KY-024 module will be ON
    // - LOW (0) when no magnet (lid open) - Blue LED on the KY-024 module will be OFF
    // - Connect the D0 (digital output) pin from the KY-024 to the ESP32 GPIO pins
    pinMode(hallSensorPins[i], INPUT);
    
    // Read initial state and display
    previousHallState[i] = digitalRead(hallSensorPins[i]);
    Serial.printf("[Setup] Hall Sensor %d initial state: %s (Reading=%d)\n", 
                 i+1, 
                 previousHallState[i] == HIGH ? "CLOSED (Magnet detected)" : "OPEN (No magnet)", 
                 previousHallState[i]);
    
    // Initialize LED state struct
    turnOffLED(i);
  }
  
  // Initialize buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); // Ensure buzzer starts silent

  // Initialize I2C for LCD and RTC (pins 21=SDA, 22=SCL as per the wiring)
  Wire.begin(21, 22);
  rtcInitialized = rtc.begin();
  
  if (rtcInitialized && rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("CareBox @MarsLab");
  lcd.setCursor(0, 1);
  lcd.print("Booting Up...");
  
  // Run direct LED test to ensure hardware is working correctly
  testRGBLEDsDirect();
  
  // Test Hall Effect sensors
  testHallSensors();
  
  // LED startup sequence - RGB test cycle for each compartment
  Serial.println("[Setup] Running quick RGB LED test sequence using LED functions");
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    Serial.printf("[Test] Compartment %d: Red\n", i+1);
    setRed(i);
    delay(200);
    
    Serial.printf("[Test] Compartment %d: Green\n", i+1);
    setGreen(i);
    delay(200);
    
    Serial.printf("[Test] Compartment %d: Blue\n", i+1);
    setBlue(i);
    delay(200);
    
    Serial.printf("[Test] Compartment %d: White\n", i+1);
    setWhite(i);
    delay(200);
    
    Serial.printf("[Test] Compartment %d: Off\n", i+1);
    turnOffLED(i);
    delay(100); // Brief pause between compartments
  }

  WiFi.begin(ssid, pass);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries++ < 20) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] Connected");
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected  ");
    
    Blynk.config(BLYNK_AUTH_TOKEN);
    for (int i = 0; i < 3; i++) {
      if (Blynk.connect(3000)) {
        lcd.setCursor(0, 1);
        lcd.print("Blynk Connected ");
        break;
      }
    }
  } else {
    Serial.println("[WiFi] Failed to connect");
    lcd.setCursor(0, 1);
    lcd.print("WiFi Failed     ");
  }

  // Set up timers
  timer.setInterval(5000L, checkAlarms);
  timer.setInterval(500L, checkHallSensors);
  timer.setInterval(100L, updateLEDs);         // Fast update for smooth blinking
  timer.setInterval(200L, handleBuzzer);
  timer.setInterval(5000L, resetDoneFlags);
  timer.setInterval(1000L, updateLCDs);        // Check LCD every second, but only update Blynk when needed
  timer.setInterval(60000L, indicateUpcomingAlarms); // Check for upcoming alarms every minute

  Serial.println("[Setup] Complete - Hall Effect Sensors ready!");
  
  // Final startup indication - all LEDs white blink once
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    setWhite(i);
  }
  delay(500);
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    turnOffLED(i);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    Blynk.run();
  }
  timer.run();
  delay(10); // Small delay for better responsiveness
}