#include <TM1637Display.h>

// 7-Segment Display Setup
TM1637Display display(2, 3); // CLK, DIO
#define BUZZER_PIN 8  // Buzzer connected to D8

#define PIR_PIN 8
#define PIR2_PIN 9
#define LED_PIN 13

// Button Pins
const int buttonMin = 4;        // Minute button
const int buttonSec = 5;        // Second button
const int buttonStartStop = 6;  // Start/Stop button


bool waitStarted = false;
bool timerDone = true;
bool timerInitialized = false;
bool timerRunning = false;

unsigned long lastSecondTime = 0;
const unsigned long interval = 1000;

unsigned long waitStartTime = 0;

int minutes = 0, seconds = 0;

bool currentStartStopButtonState = HIGH;
bool lastStartStopButtonState = HIGH;

bool minButtonIsPressed = false;
bool minButtonWasPressed = false;

bool secButtonIsPressed = false;
bool secButtonWasPressed = false;

bool timerReset = false;

bool flashDisplayEnabled = false;
bool showColin = true;
bool flashDisplayInitialized = false;

unsigned long flashStartTime = 0;

void setup() {
  Serial.begin(9600);
  display.setBrightness(2);

  pinMode(buttonMin, INPUT_PULLUP);
  pinMode(buttonSec, INPUT_PULLUP);
  pinMode(buttonStartStop, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Ensure it's off at the start

  
  pinMode(PIR_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // minutes = 99;
  // seconds = 11;

  updateDisplay();
}

void loop() {

int motion = digitalRead(PIR_PIN);
int motion2 = digitalRead(PIR2_PIN);
  
if (motion == HIGH) { // Motion detected
  Serial.println("Motion detected!");
  digitalWrite(LED_PIN, HIGH);
} else {
  digitalWrite(LED_PIN, LOW);
}

if (motion2 == HIGH) { // Motion detected
  Serial.println("Motion2 detected!");
  digitalWrite(LED_PIN, HIGH);
} else {
  digitalWrite(LED_PIN, LOW);
}

delay(10); // Small delay to prevent rapid toggling


  // if (startStopButtonPressed()) {
  //   if (!timerDone) {
  //     timerRunning = !timerRunning;
  //   } else if (!flashDisplayEnabled) {
  //     if (minutes > 0 || seconds > 0) {
  //       timerRunning = !timerRunning;
  //     }
  //   }
  // }

  // if (timerReset) {
  //   if (!minButtonPressed() && !secButtonPressed()) {
  //     timerReset = false;
  //   }
  // } else if (!timerRunning) {
  //   if (minButtonPressed() && secButtonPressed()) {
  //     timerReset = true;
  //     minutes = 0;
  //     seconds = 0;
  //     updateDisplay();
  //   } else {
  //     increaseTimer();
  //     updateDisplay();
  //   }
  // }

  // if (flashDisplayEnabled) {
  //   if (digitalRead(buttonStartStop) == LOW || minButtonPressed() || secButtonPressed()) {
  //     flashDisplayEnabled = false;
  //     flashDisplayInitialized = false;
  //     showColin = true;
  //   }
  //   flashDisplay();
  // }

  // // Serial.println("timerRunning: " + String(timerRunning) + ", timerDone: " + String(timerDone) + ", flashDisplayEnabled: " + String(flashDisplayEnabled));

  // runTimer();
}

bool waitIter(unsigned long x) {
  if (!waitStarted) {
    waitStartTime = millis();
    waitStarted = true;
  } else if (millis() - waitStartTime >= x) {
    waitStarted = false;
    return true;
  }
}

void flashDisplay() {
  if (!flashDisplayInitialized) {
    flashStartTime = millis();
    flashDisplayInitialized = true;
  }

  if (millis() - flashStartTime > 250) {
    showColin = !showColin;
    flashStartTime = millis();
  }
}

void runTimer() {
  if (timerRunning) {
    if (!timerInitialized) {
      lastSecondTime = millis();
      timerInitialized = true;
      timerDone = false;
    }

    if (!timerDone && (millis() - lastSecondTime) >= interval) {
      lastSecondTime += interval;
      if (seconds > 0) {  // there are seconds left
        seconds--;
      } else if (minutes > 0) {  // there are no seconds left, but there are minutes left
        minutes--;
        seconds = 59;
      } else {  // there are no seconds or minutes left
        timerDone = true;
        timerInitialized = false;
        flashDisplayEnabled = true;
        timerRunning = false;
      }
    }
    updateDisplay();
  } else { // timerRunning is false
    lastSecondTime = millis();
  }
}

void updateDisplay() {
  if (showColin) {
    display.showNumberDecEx((minutes * 100) + seconds, 0b11100000, true);
  } else {
    display.showNumberDecEx((minutes * 100) + seconds, 0b00000000, true);
  }
}

bool minButtonPressed() {
  return digitalRead(buttonMin) == LOW ? true : false;
}

bool secButtonPressed() {
  return digitalRead(buttonSec) == LOW ? true : false;
}

void increaseTimer() {
  minButtonIsPressed = minButtonPressed();
  secButtonIsPressed = secButtonPressed();
  if (!flashDisplayEnabled) {
    if (minButtonIsPressed && !minButtonWasPressed) {
      minButtonWasPressed = minButtonIsPressed;
      minutes++;
    } else {
      minButtonWasPressed = minButtonIsPressed;
    }

    if (secButtonIsPressed && !secButtonWasPressed) {
      secButtonWasPressed = secButtonIsPressed;
      seconds++;
    } else {
      secButtonWasPressed = secButtonIsPressed;
    }
  } else {
      minButtonWasPressed = minButtonIsPressed;
      secButtonWasPressed = secButtonIsPressed;
  }
}

bool startStopButtonPressed() {
  currentStartStopButtonState = digitalRead(buttonStartStop);

  if (currentStartStopButtonState == LOW && lastStartStopButtonState == HIGH && !flashDisplayEnabled) {
    lastStartStopButtonState = currentStartStopButtonState;
    return true;
  } else {
    lastStartStopButtonState = currentStartStopButtonState;
    return false;
  }
}