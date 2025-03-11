#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TM1637Display.h>

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
bool buzzerState = false;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("Timer: 00:00");

  display.setBrightness(7);

  pinMode(buttonMin, INPUT_PULLUP);
  pinMode(buttonSec, INPUT_PULLUP);
  pinMode(buttonStartStop, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Ensure it's off at the start
}

void loop() {
  // Button Handling
  if (!timerRunning && !alarmActive) {  // Adjust time only if timer is stopped and no alarm
    if (digitalRead(buttonMin) == LOW) {  
      minutes = min(90, minutes + 1);
      delay(150);
    }

    if (digitalRead(buttonSec) == LOW) {  
      seconds = (seconds + 1) % 60;
      delay(150);
    }

    // Reset if both buttons are pressed
    if (digitalRead(buttonMin) == LOW && digitalRead(buttonSec) == LOW) {
      minutes = 0;
      seconds = 0;
      delay(500);
    }
  }

  // Start/Stop button
  if (digitalRead(buttonStartStop) == LOW && !alarmActive) {
    timerRunning = !timerRunning;
    lastSecondTime = millis();
    delay(300);  
  }

  // Countdown logic
  if (timerRunning && millis() - lastSecondTime >= interval) {
    lastSecondTime += interval;

    if (seconds == 0) {
      if (minutes > 0) {
        minutes--;
        seconds = 59;
      } else {
        // Timer reached zero, start alarm
        timerRunning = false;
        alarmActive = true;
        lastBeepTime = millis();
      }
    } else {
      seconds--;
    }
  }

  // Handle alarm (buzzer + flashing LCD + flashing 7-segment)
  if (alarmActive) {
    if (millis() - lastBeepTime >= 500) { // Toggle every 500ms
      lastBeepTime = millis();
      buzzerState = !buzzerState;
      
      // Toggle buzzer, LCD, and 7-segment display
      digitalWrite(BUZZER_PIN, buzzerState);
      buzzerState ? lcd.noBacklight() : lcd.backlight();

      if (buzzerState) {
        display.clear();  // "Turn off" the 7-segment display
      } else {
        int timeValue = (minutes * 100) + seconds;
        display.showNumberDecEx(timeValue, 0b11100000, true);  // Restore time
      }
    }

    // Stop alarm when any button is pressed
    if (digitalRead(buttonMin) == LOW || digitalRead(buttonSec) == LOW || digitalRead(buttonStartStop) == LOW) {
      alarmActive = false;
      digitalWrite(BUZZER_PIN, LOW);
      lcd.backlight();
      display.setBrightness(7);
      int timeValue = (minutes * 100) + seconds;
      display.showNumberDecEx(timeValue, 0b11100000, true); // Ensure it's back on
      delay(300);  // Simple debounce
    }
  }

  // Update LCD and 7-Segment Display (only if alarm is not active)
  if (!alarmActive) {
    lcd.setCursor(9, 0);
    lcd.print(minutes < 10 ? "0" : "");
    lcd.print(minutes);
    lcd.print(":");
    lcd.print(seconds < 10 ? "0" : "");
    lcd.print(seconds);

    int timeValue = (minutes * 100) + seconds;
    display.showNumberDecEx(timeValue, 0b11100000, true);
  }
}