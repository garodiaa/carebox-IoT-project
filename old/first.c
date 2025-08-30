#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6frQ21qaS"
#define BLYNK_TEMPLATE_NAME "CareBox"
#define BLYNK_AUTH_TOKEN "Ctxz2DA5sUwh9tYBT5r3kRteCxmOHeSh"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <climits>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int NUM_COMPARTMENTS = 3;
const int ALARMS_PER_COMP = 3;
const int ledPins[NUM_COMPARTMENTS] = {13, 12, 14};
const int reedPins[NUM_COMPARTMENTS] = {25, 26, 27};
const int buzzerPin = 33;
bool previousButtonState[NUM_COMPARTMENTS] = {HIGH, HIGH, HIGH};

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
bool pulseState = false;
unsigned long lastPulseTime = 0;
const unsigned long pulseInterval = 500;
int rtcErrorCount = 0;
const int MAX_RTC_ERRORS = 5;

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
    for (int d = 0; d < 7; d++) {
      a.daysOfWeek[d] = t.isWeekdaySelected(d);
      Serial.printf("  Day %d selected: %d\n", d, a.daysOfWeek[d]);
    }
  }
}

BLYNK_CONNECTED() {
  Serial.println("[Blynk] Connected, syncing virtual pins...");
  Blynk.syncVirtual(V0, V1, V2);
  Blynk.syncVirtual(V3, V4, V5, V6, V7, V8, V9, V10, V11);
}

BLYNK_WRITE(V0) { strncpy(schedules[0].medicineName, param.asString(), 19); schedules[0].medicineName[19] = '\0'; Serial.printf("[Blynk] V0 - Medicine Name 1: %s\n", schedules[0].medicineName); }
BLYNK_WRITE(V1) { strncpy(schedules[1].medicineName, param.asString(), 19); schedules[1].medicineName[19] = '\0'; Serial.printf("[Blynk] V1 - Medicine Name 2: %s\n", schedules[1].medicineName); }
BLYNK_WRITE(V2) { strncpy(schedules[2].medicineName, param.asString(), 19); schedules[2].medicineName[19] = '\0'; Serial.printf("[Blynk] V2 - Medicine Name 3: %s\n", schedules[2].medicineName); }

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
  Serial.printf("[Time] Checking alarms at %02d:%02d (Weekday %d)\n", h, m, wd);

  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    for (int j = 0; j < ALARMS_PER_COMP; j++) {
      Alarm& a = schedules[i].alarms[j];
      if (a.enabled && a.hour == h && a.minute == m && a.daysOfWeek[wd] &&
          !schedules[i].reminderActive[j] && !schedules[i].reminderDone[j]) {
        schedules[i].reminderActive[j] = true;
        Serial.printf("[Alarm] Triggered for %s (Comp %d, Alarm %d)\n", schedules[i].medicineName, i + 1, j + 1);
        if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
          char msg[50];
          snprintf(msg, sizeof(msg), "Time for %s", schedules[i].medicineName);
          Blynk.logEvent("reminder", msg);
        }
      }
    }
  }
}

void handleLEDsAndBuzzer() {
  bool anyActive = false;
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    for (int j = 0; j < ALARMS_PER_COMP; j++) {
      if (schedules[i].reminderActive[j]) anyActive = true;
    }
  }
  if (anyActive) {
    if (millis() - lastPulseTime >= pulseInterval) {
      pulseState = !pulseState;
      lastPulseTime = millis();
    }
    digitalWrite(buzzerPin, pulseState);
    for (int i = 0; i < NUM_COMPARTMENTS; i++) {
      bool on = false;
      for (int j = 0; j < ALARMS_PER_COMP; j++) {
        if (schedules[i].reminderActive[j]) on = true;
      }
      digitalWrite(ledPins[i], pulseState && on);
    }
  } else {
    digitalWrite(buzzerPin, LOW);
    for (int i = 0; i < NUM_COMPARTMENTS; i++) digitalWrite(ledPins[i], LOW);
  }
}

void checkButtons() {
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    int state = digitalRead(reedPins[i]);
    if (previousButtonState[i] == HIGH && state == LOW) {
      Serial.printf("[Input] Button press detected on Compartment %d\n", i + 1);
      for (int j = 0; j < ALARMS_PER_COMP; j++) {
        if (schedules[i].reminderActive[j]) {
          schedules[i].reminderActive[j] = false;
          schedules[i].reminderDone[j] = true;
          Serial.printf("[Alarm] Reminder cleared for %s\n", schedules[i].medicineName);
        }
      }
      if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
        Blynk.virtualWrite(V12, String(schedules[i].medicineName) + " taken");
      }
    }
    previousButtonState[i] = state;
  }
}

void resetDoneFlags() {
  if (!safeReadRTC()) return;
  if (currentTime.minute() != lastMinute) {
    for (int i = 0; i < NUM_COMPARTMENTS; i++)
      for (int j = 0; j < ALARMS_PER_COMP; j++)
        schedules[i].reminderDone[j] = false;
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
  
  // Format the time remaining as "Next: In XXh XXm"
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
  snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d:%02d", currentTime.hour(), currentTime.minute(), currentTime.second());

  // Update physical LCD
  lcd.setCursor(0, 0);
  lcd.print(reminderText);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(timeStr);
  lcd.print("                ");

  // Update Blynk LCD in Simple Mode
  if (Blynk.connected()) {
    Blynk.virtualWrite(V12, reminderText); // First line
    Blynk.virtualWrite(V13, timeStr);      // Second line
  }

  Serial.printf("[LCD Sync] %s | %s\n", reminderText.c_str(), timeStr);
}

void sendStatusToBlynk() {
  if (WiFi.status() != WL_CONNECTED || !Blynk.connected()) return;
  if (!safeReadRTC()) {
    return;
  }
  int activeCount = 0;
  for (int i = 0; i < NUM_COMPARTMENTS; i++)
    for (int j = 0; j < ALARMS_PER_COMP; j++)
      if (schedules[i].reminderActive[j]) activeCount++;
  
  String statusMsg;
  if (activeCount > 0) {
    statusMsg = String(activeCount) + " reminders active";
  } else {
    statusMsg = getNextAlarmText();
  }
  Blynk.virtualWrite(V14, statusMsg);
  Serial.printf("[Status] Sent to Blynk - Status: %s\n", statusMsg.c_str());
}

void setup() {
  Serial.begin(115200);
  Serial.println("[Setup] Starting...");
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(reedPins[i], INPUT_PULLUP);
  }
  pinMode(buzzerPin, OUTPUT);

  Wire.begin(21, 22);
  rtcInitialized = rtc.begin();
  if (rtcInitialized && rtc.lostPower()) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("CareBox | MarsLab");

  WiFi.begin(ssid, pass);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries++ < 20) delay(500);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] Connected");
    Blynk.config(BLYNK_AUTH_TOKEN);
    for (int i = 0; i < 3; i++) if (Blynk.connect(3000)) break;
  } else {
    Serial.println("[WiFi] Failed to connect");
  }

  timer.setInterval(5000L, checkAlarms);
  timer.setInterval(500L, checkButtons);
  timer.setInterval(200L, handleLEDsAndBuzzer);
  timer.setInterval(5000L, resetDoneFlags);
  timer.setInterval(1000L, updateLCDs);
  timer.setInterval(15000L, sendStatusToBlynk);

  Serial.println("[Setup] Complete");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) Blynk.run();
  timer.run();
  delay(50);
}
