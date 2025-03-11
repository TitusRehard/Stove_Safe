#include <TM1637Display.h>

// 7-Segment Display Setup
#define CLK 3  
#define DIO 4  
TM1637Display display(CLK, DIO);
#define BUZZER_PIN 8  // Buzzer connected to D8

// Button Pins
const int buttonMin = 5;  // Minute button
const int buttonSec = 6;  // Second button
const int buttonStartStop = 7;  // Start/Stop button

// Global timer variables
unsigned long lastSecondTime = 0;
const unsigned long interval = 1000;
int minutes = 0, seconds = 0;
bool timerRunning = false;
bool alarmActive = false;
unsigned long lastBeepTime = 0;
unsigned long alarmCycleTime = 0;
bool buzzerState = false;
unsigned long beepInterval = 500;  // Interval for buzzer on/off (500ms)
unsigned long flashInterval = 500;  // Interval for display flashing (500ms)
unsigned long lastDisplayUpdate = 0;  // Track display update time
unsigned long lastButtonPressTime = 0;  // To avoid bouncing

void setup() {
  display.setBrightness(7);

  pinMode(buttonMin, INPUT_PULLUP);
  pinMode(buttonSec, INPUT_PULLUP);
  pinMode(buttonStartStop, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Ensure it's off at the start
}

void loop() {
  unsigned long currentMillis = millis();

  // Button Handling - Ignore button presses if timer is running or alarm is active
  if (!timerRunning && !alarmActive) {  
    if (digitalRead(buttonMin) == LOW && currentMillis - lastButtonPressTime > 150) {  // Simple debounce
      minutes = min(90, minutes + 1);
      lastButtonPressTime = currentMillis;
    }

    if (digitalRead(buttonSec) == LOW && currentMillis - lastButtonPressTime > 150) {
      seconds = (seconds + 1) % 60;
      lastButtonPressTime = currentMillis;
    }

    // Reset if both buttons are pressed
    if (digitalRead(buttonMin) == LOW && digitalRead(buttonSec) == LOW && currentMillis - lastButtonPressTime > 500) {
      minutes = 0;
      seconds = 0;
      lastButtonPressTime = currentMillis;
    }
  }

  // Start/Stop button
  if (digitalRead(buttonStartStop) == LOW && currentMillis - lastButtonPressTime > 300 && !alarmActive) {
    timerRunning = !timerRunning;
    lastSecondTime = currentMillis;
    lastButtonPressTime = currentMillis;  
  }

  // Countdown logic
  if (timerRunning && currentMillis - lastSecondTime >= interval) {
    lastSecondTime = currentMillis;

    if (seconds == 0) {
      if (minutes > 0) {
        minutes--;
        seconds = 59;
      } else {
        // Timer reached zero, start alarm
        timerRunning = false;
        alarmActive = true;
        lastBeepTime = currentMillis;
        alarmCycleTime = currentMillis;  // Reset alarm cycle timer
      }
    } else {
      seconds--;
    }

    // Update display immediately after decrement
    int timeValue = (minutes * 100) + seconds;
    display.showNumberDecEx(timeValue, 0b11100000, true);
  }

  // Handle alarm (buzzer + flashing 7-segment)
  if (alarmActive) {
    if (currentMillis - alarmCycleTime >= beepInterval) {
      buzzerState = !buzzerState;

      if (buzzerState) {
        digitalWrite(BUZZER_PIN, HIGH);  // Turn on buzzer
        display.clear();  // "Turn off" the 7-segment display during beep
      } else {
        digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer
        display.showNumberDecEx(0, 0b11100000, true);  // Turn on 7-segment display with a placeholder
      }

      alarmCycleTime = currentMillis;  // Reset alarm cycle timer
    }

    // Flash the display
    if (currentMillis - lastDisplayUpdate >= flashInterval) {
      lastDisplayUpdate = currentMillis;

      if (buzzerState) {
        display.showNumberDecEx(0, 0b11100000, true);  // Flash display
      } else {
        display.showNumberDecEx(0, 0b11100000, true);  // Turn on placeholder during beep
      }
    }

    // Stop alarm when any button is pressed
    if (digitalRead(buttonMin) == LOW || digitalRead(buttonSec) == LOW || digitalRead(buttonStartStop) == LOW) {
      alarmActive = false;
      digitalWrite(BUZZER_PIN, LOW);
      display.setBrightness(7);
      int timeValue = (minutes * 100) + seconds;
      display.showNumberDecEx(timeValue, 0b11100000, true); // Ensure it's back on
      lastButtonPressTime = currentMillis;  // Reset debounce timer
    }
  }

  // Update 7-Segment Display (only if alarm is not active)
  if (!alarmActive && currentMillis - lastDisplayUpdate >= flashInterval) {
    lastDisplayUpdate = currentMillis;
    int timeValue = (minutes * 100) + seconds;
    display.showNumberDecEx(timeValue, 0b11100000, true);
  }
}