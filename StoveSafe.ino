#include <TM1637Display.h>

// 7-Segment Display Setup
TM1637Display display(2, 3); // CLK, DIO

#define BUZZER_PIN 5
#define PIR_PIN 6
#define MIN_PIN 9
#define SEC_PIN 8
#define START_PIN 7

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

int flashState = 0;
int alarmState = 0;

bool enableBuzzer = false;

unsigned long lastMotionCheck = 0;
unsigned long lastMotionDetected = 0;

bool enableAlarm = false;

void setup() {
  Serial.begin(9600);
  display.setBrightness(2);

  pinMode(MIN_PIN, INPUT_PULLUP);
  pinMode(SEC_PIN, INPUT_PULLUP);
  pinMode(START_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Ensure it's off at the start

  
  pinMode(PIR_PIN, INPUT);

  lastMotionDetected = millis();

  // minutes = 10;
  // seconds = 0;
  // timerRunning = true;

  updateDisplay();
}

void loop() {

  if (timerRunning) { // see if there is motion
    if (millis() - lastMotionCheck >= 300) {
      lastMotionCheck = millis();
      if (detectMotion()) {
        lastMotionDetected = millis();
        showColin = false;
      } else {
        showColin = true;
      }
    }

    if (millis() - lastMotionDetected > 15000) {
      enableAlarm = true;
    }
  }

// int motion = digitalRead(PIR_PIN);
  
// if (motion == HIGH) { // Motion detected
//   Serial.println("Motion detected!");
//   digitalWrite(LED_PIN, HIGH);
// } else {
//   digitalWrite(LED_PIN, LOW);
// }

// delay(10); // Small delay to prevent rapid toggling


  

  if (enableAlarm) {
    beepAlarm();
    if (digitalRead(START_PIN) == LOW || minButtonPressed() || secButtonPressed()) {
      flashDisplayEnabled = false;
      flashDisplayInitialized = false;
      showColin = true;
      alarmState = 0;
      enableBuzzer = false;
      enableAlarm = false;
      lastStartStopButtonState = LOW;
      lastMotionDetected = millis();
    }
  } else if (timerReset) {
    if (!minButtonPressed() && !secButtonPressed()) {
      timerReset = false;
    }
  } else if (!timerRunning) {
    if (minButtonPressed() && secButtonPressed()) {
      timerReset = true;
      minutes = 0;
      seconds = 0;
      updateDisplay();
    } else {
      increaseTimer();
      updateDisplay();
    }
  }

  if (startStopButtonPressed()) {
    if (!timerDone) {
      timerRunning = !timerRunning;
      lastMotionDetected = millis();
    } else if (!flashDisplayEnabled) {
      if (minutes > 0 || seconds > 0) {
        timerRunning = !timerRunning;
        lastMotionDetected = millis();
      }
    }
  }

  if (flashDisplayEnabled) {
    beepAlarm();
    if (digitalRead(START_PIN) == LOW) {// || minButtonPressed() || secButtonPressed()) {
      flashDisplayEnabled = false;
      flashDisplayInitialized = false;
      showColin = true;
      flashState = 0;
      enableBuzzer = false;
    }
  }

  // Serial.println("lastStartStopButtonState: " + String(lastStartStopButtonState) + ", currentStartStopButtonState: " + String(currentStartStopButtonState));
  // Serial.println("timerRunning: " + String(timerRunning) + ", timerDone: " + String(timerDone) + ", flashDisplayEnabled: " + String(flashDisplayEnabled));
  // Serial.println("flashState: " + String(flashState));

  runTimer();
  runBuzzer();
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

void beepAlarm() {
  if (!enableAlarm) {
    switch (flashState) {
      case 0: // off short
        if (!flashDisplayInitialized) {
          flashStartTime = millis();
          flashDisplayInitialized = true;
          showColin = false;
          enableBuzzer = true;
        }
        if (millis() - flashStartTime > 100) {
          flashDisplayInitialized = false;
          flashStartTime = millis();
          flashState = 1;
        }
        break;
      case 1: // on short
        if (!flashDisplayInitialized) {
          flashStartTime = millis();
          flashDisplayInitialized = true;
          showColin = true;
          enableBuzzer = false;
        }
        if (millis() - flashStartTime > 150) {
          flashDisplayInitialized = false;
          flashStartTime = millis();
          flashState = 2;
        }
        break;
      case 2: // off short
        if (!flashDisplayInitialized) {
          flashStartTime = millis();
          flashDisplayInitialized = true;
          showColin = false;
          enableBuzzer = true;
        }
        if (millis() - flashStartTime > 100) {
          flashDisplayInitialized = false;
          flashStartTime = millis();
          flashState = 3;
        }
        break;
      case 3: // on long
        if (!flashDisplayInitialized) {
          flashStartTime = millis();
          flashDisplayInitialized = true;
          showColin = true;
          enableBuzzer = false;
        }
        if (millis() - flashStartTime > 1200) {
          flashDisplayInitialized = false;
          flashStartTime = millis();
          flashState = 0;
        }
        break;
    }
  } else {
    switch (alarmState) {
      case 0: // on
        if (!flashDisplayInitialized) {
          flashStartTime = millis();
          flashDisplayInitialized = true;
          // showColin = false;
          enableBuzzer = true;
        }
        if (millis() - flashStartTime > 1000) {
          flashDisplayInitialized = false;
          flashStartTime = millis();
          alarmState = 1;
        }
        break;
      case 1: // off
        if (!flashDisplayInitialized) {
          flashStartTime = millis();
          flashDisplayInitialized = true;
          // showColin = true;
          enableBuzzer = false;
        }
        if (millis() - flashStartTime > 1000) {
          flashDisplayInitialized = false;
          flashStartTime = millis();
          alarmState = 0;
        }
        break;
    }
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
  return digitalRead(MIN_PIN) == LOW ? true : false;
}

bool secButtonPressed() {
  return digitalRead(SEC_PIN) == LOW ? true : false;
}

void increaseTimer() {
  minButtonIsPressed = minButtonPressed();
  secButtonIsPressed = secButtonPressed();
  if (!flashDisplayEnabled) {
    if (minButtonIsPressed && !minButtonWasPressed) {
      minButtonWasPressed = minButtonIsPressed;
      minutes++;
      if (minutes > 90) {
        minutes = 0;
      }
    } else {
      minButtonWasPressed = minButtonIsPressed;
    }

    if (secButtonIsPressed && !secButtonWasPressed) {
      secButtonWasPressed = secButtonIsPressed;
      seconds++;
      if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes > 90) {
          minutes = 0;
        }
      }
    } else {
      secButtonWasPressed = secButtonIsPressed;
    }
  } else {
      minButtonWasPressed = minButtonIsPressed;
      secButtonWasPressed = secButtonIsPressed;
  }
}

bool startStopButtonPressed() {
  currentStartStopButtonState = digitalRead(START_PIN);

  if (currentStartStopButtonState == LOW && lastStartStopButtonState == HIGH && !flashDisplayEnabled) {
    lastStartStopButtonState = currentStartStopButtonState;
    // Serial.println("lastStartStopButtonState: " + String(lastStartStopButtonState) + ", currentStartStopButtonState: " + String(currentStartStopButtonState));
    return true;
  } else {
    lastStartStopButtonState = currentStartStopButtonState;
    return false;
  }
}

void runBuzzer() {
  if (enableBuzzer) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

bool detectMotion() {
  if (digitalRead(PIR_PIN) == HIGH) { // Motion detected
    // Serial.println("Motion detected!");
    return true;
  } else {
    // Serial.println("No motion detected...");
    return false;
  }
}