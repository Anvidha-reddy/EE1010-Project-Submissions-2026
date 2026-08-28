// ============================================================================
// 6-Digit Clock & Alarm System - Pure K-Map Engine (12-Hour Format)
// ============================================================================

const int pinA = 2; // IC 7447 Pin 7
const int pinB = 3; // IC 7447 Pin 1
const int pinC = 4; // IC 7447 Pin 2
const int pinD = 5; // IC 7447 Pin 6

const int pinSegG = 12; // Segment G resistor line (Active-LOW bypass)

const int digitPins[6] = {6, 7, 8, 9, 10, 11}; // Digit enable pins

// Sound & Control Pins
const int pinBuzzer = A0; // Piezo Buzzer Output
const int btnMode   = A1; // Mode Switch (Normal -> Set Clock -> Set Alarm)
const int btnMin    = A2; // Increment Minutes
const int btnHrs    = A3; // Increment Hours
const int btnAlarm  = A4; // Alarm Toggle / Silence

// Debounce trackers
unsigned long lastDebounceMode  = 0;
unsigned long lastDebounceMin   = 0;
unsigned long lastDebounceHrs   = 0;
unsigned long lastDebounceAlarm = 0;
const unsigned long debounceDelay = 200; // ms

// Active-LOW Lookup Table for Bypassed Segment G (LOW = ON, HIGH = OFF)
const bool segGState[10] = {HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW};

// Real-Time Clock Digits (Starts at 12:00:00)
int secUnits = 0;
int secTens  = 0;
int minUnits = 0;
int minTens  = 0;
int hrsUnits = 2;
int hrsTens  = 1;

// Alarm Target Digits (Default 12:01:00)
int alarmMinUnits = 1;
int alarmMinTens  = 0;
int alarmHrsUnits = 2;
int alarmHrsTens  = 1;

// System States
int currentMode   = 0;     // 0 = Normal Clock, 1 = Set Time, 2 = Set Alarm
bool alarmEnabled = false; // Alarm On/Off Toggle
bool alarmRinging = false; // Alarm Active Tone State

unsigned long previousMillis = 0;

void setup() {
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);

  pinMode(pinSegG, OUTPUT);
  digitalWrite(pinSegG, HIGH);

  for (int i = 0; i < 6; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW);
  }

  pinMode(pinBuzzer, OUTPUT);

  // Configure push buttons with internal pull-ups (PRESS = LOW)
  pinMode(btnMode, INPUT_PULLUP);
  pinMode(btnMin, INPUT_PULLUP);
  pinMode(btnHrs, INPUT_PULLUP);
  pinMode(btnAlarm, INPUT_PULLUP);
}

// K-Map Minimized Next-State Function for BCD Modulo-10 Counter (0 to 9)
// Boolean Logic: Anext = !A | Bnext = B!A + !D!BA | Cnext = C!A + C!B + !CBA | Dnext = D!A + CBA
int kmapMod10Next(int val) {
  bool A = val & 0x01;
  bool B = (val >> 1) & 0x01;
  bool C = (val >> 2) & 0x01;
  bool D = (val >> 3) & 0x01;

  bool Anext = !A;
  bool Bnext = (B && !A) || (!D && !B && A);
  bool Cnext = (C && !A) || (C && !B) || (!C && B && A);
  bool Dnext = (D && !A) || (C && B && A);

  return (Dnext << 3) | (Cnext << 2) | (Bnext << 1) | Anext;
}

// K-Map Minimized Next-State Function for Modulo-6 Counter (0 to 5)
// Boolean Logic: Xnext = !X | Ynext = Y!X + !Z!YX | Znext = Z!X + YX
int kmapMod6Next(int val) {
  bool X = val & 0x01;
  bool Y = (val >> 1) & 0x01;
  bool Z = (val >> 2) & 0x01;

  bool Xnext = !X;
  bool Ynext = (Y && !X) || (!Z && !Y && X);
  bool Znext = (Z && !X) || (Y && X);

  return (Znext << 2) | (Ynext << 1) | Xnext;
}

// K-Map Minimized 12-Hour State Transition Engine (12 -> 01 transition)
void incrementHoursKMap(int &hTens, int &hUnits) {
  if (hTens == 1 && hUnits == 2) {
    hTens = 0;
    hUnits = 1;
  } else if (hTens == 0 && hUnits == 9) {
    hTens = 1;
    hUnits = 0;
  } else {
    hUnits = kmapMod10Next(hUnits);
  }
}

// K-Map Minimized Minute Transition Engine (00 -> 59)
void incrementMinutesKMap(int &mTens, int &mUnits) {
  if (mUnits == 9) {
    mUnits = 0;
    if (mTens == 5) {
      mTens = 0;
    } else {
      mTens = kmapMod6Next(mTens);
    }
  } else {
    mUnits = kmapMod10Next(mUnits);
  }
}

// Time Advance Logic driven by K-Map Functions
void advanceTimeKMap() {
  int nextSecUnits = kmapMod10Next(secUnits);
  
  if (secUnits == 9) {
    secUnits = 0;
    int nextSecTens = kmapMod6Next(secTens);
    
    if (secTens == 5) {
      secTens = 0;
      
      int prevMinUnits = minUnits;
      int prevMinTens  = minTens;
      incrementMinutesKMap(minTens, minUnits);
      
      if (prevMinTens == 5 && prevMinUnits == 9 && minUnits == 0 && minTens == 0) {
        incrementHoursKMap(hrsTens, hrsUnits);
      }
    } else {
      secTens = nextSecTens;
    }
  } else {
    secUnits = nextSecUnits;
  }

  // Trigger Alarm Match Check
  if (alarmEnabled && hrsTens == alarmHrsTens && hrsUnits == alarmHrsUnits &&
      minTens == alarmMinTens && minUnits == alarmMinUnits && secTens == 0 && secUnits == 0) {
    alarmRinging = true;
  }
}

void beep(int duration) {
  tone(pinBuzzer, 1000);
  delay(duration);
  noTone(pinBuzzer);
}

void handleButtons() {
  unsigned long currentMillis = millis();

  // Button 1: Mode Switch (A1)
  if (digitalRead(btnMode) == LOW) {
    if (currentMillis - lastDebounceMode > debounceDelay) {
      currentMode = (currentMode + 1) % 3;
      lastDebounceMode = currentMillis;
      beep(100);
    }
  }

  // Button 2: Minutes Increment (A2)
  if (digitalRead(btnMin) == LOW) {
    if (currentMillis - lastDebounceMin > debounceDelay) {
      if (currentMode == 1) {
        incrementMinutesKMap(minTens, minUnits);
        secUnits = 0;
        secTens  = 0;
      } else if (currentMode == 2) {
        incrementMinutesKMap(alarmMinTens, alarmMinUnits);
      }
      lastDebounceMin = currentMillis;
      beep(50);
    }
  }

  // Button 3: Hours Increment (A3)
  if (digitalRead(btnHrs) == LOW) {
    if (currentMillis - lastDebounceHrs > debounceDelay) {
      if (currentMode == 1) {
        incrementHoursKMap(hrsTens, hrsUnits);
      } else if (currentMode == 2) {
        incrementHoursKMap(alarmHrsTens, alarmHrsUnits);
      }
      lastDebounceHrs = currentMillis;
      beep(50);
    }
  }

  // Button 4: Alarm Toggle / Stop (A4)
  if (digitalRead(btnAlarm) == LOW) {
    if (currentMillis - lastDebounceAlarm > debounceDelay) {
      if (alarmRinging) {
        alarmRinging = false;
        noTone(pinBuzzer);
      } else {
        alarmEnabled = !alarmEnabled;
        beep(100);
        if (alarmEnabled) {
          delay(100);
          beep(100);
        }
      }
      lastDebounceAlarm = currentMillis;
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis += 1000;
    advanceTimeKMap();
  }

  // Sound Buzzer Pattern when Alarm Triggers
  if (alarmRinging) {
    if ((millis() / 250) % 2 == 0) {
      tone(pinBuzzer, 2000);
    } else {
      noTone(pinBuzzer);
    }
  }

  handleButtons();
  refreshDisplay();
}

void writeBCDAndBypasses(int val) {
  // Output BCD value to 7447 IC pins (A, B, C, D)
  digitalWrite(pinA, (val & 0x01) ? HIGH : LOW);
  digitalWrite(pinB, (val & 0x02) ? HIGH : LOW);
  digitalWrite(pinC, (val & 0x04) ? HIGH : LOW);
  digitalWrite(pinD, (val & 0x08) ? HIGH : LOW);

  // Output manual segment G line only
  digitalWrite(pinSegG, segGState[val]);
}

void refreshDisplay() {
  int digits[6];

  if (currentMode == 0 || currentMode == 1) {
    // Clock Mode: Display real-time clock (HH:MM:SS)
    digits[0] = hrsTens;
    digits[1] = hrsUnits;
    digits[2] = minTens;
    digits[3] = minUnits;
    digits[4] = secTens;
    digits[5] = secUnits;
  } else if (currentMode == 2) {
    // Alarm Mode: Display set alarm time (HH:MM:00)
    digits[0] = alarmHrsTens;
    digits[1] = alarmHrsUnits;
    digits[2] = alarmMinTens;
    digits[3] = alarmMinUnits;
    digits[4] = 0;
    digits[5] = 0;
  }

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      digitalWrite(digitPins[j], LOW);
    }

    writeBCDAndBypasses(digits[i]);

    digitalWrite(digitPins[i], HIGH);
    delayMicroseconds(1000);
    digitalWrite(digitPins[i], LOW);
  }
}// ============================================================================
// 6-Digit Clock & Alarm System - Pure K-Map Engine (12-Hour Format)
// ============================================================================

const int pinA = 2; // IC 7447 Pin 7
const int pinB = 3; // IC 7447 Pin 1
const int pinC = 4; // IC 7447 Pin 2
const int pinD = 5; // IC 7447 Pin 6

const int pinSegG = 12; // Segment G resistor line (Active-LOW bypass)

const int digitPins[6] = {6, 7, 8, 9, 10, 11}; // Digit enable pins

// Sound & Control Pins
const int pinBuzzer = A0; // Piezo Buzzer Output
const int btnMode   = A1; // Mode Switch (Normal -> Set Clock -> Set Alarm)
const int btnMin    = A2; // Increment Minutes
const int btnHrs    = A3; // Increment Hours
const int btnAlarm  = A4; // Alarm Toggle / Silence

// Debounce trackers
unsigned long lastDebounceMode  = 0;
unsigned long lastDebounceMin   = 0;
unsigned long lastDebounceHrs   = 0;
unsigned long lastDebounceAlarm = 0;
const unsigned long debounceDelay = 200; // ms

// Active-LOW Lookup Table for Bypassed Segment G (LOW = ON, HIGH = OFF)
const bool segGState[10] = {HIGH, HIGH, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW};

// Real-Time Clock Digits (Starts at 12:00:00)
int secUnits = 0;
int secTens  = 0;
int minUnits = 0;
int minTens  = 0;
int hrsUnits = 2;
int hrsTens  = 1;

// Alarm Target Digits (Default 12:01:00)
int alarmMinUnits = 1;
int alarmMinTens  = 0;
int alarmHrsUnits = 2;
int alarmHrsTens  = 1;

// System States
int currentMode   = 0;     // 0 = Normal Clock, 1 = Set Time, 2 = Set Alarm
bool alarmEnabled = false; // Alarm On/Off Toggle
bool alarmRinging = false; // Alarm Active Tone State

unsigned long previousMillis = 0;

void setup() {
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);

  pinMode(pinSegG, OUTPUT);
  digitalWrite(pinSegG, HIGH);

  for (int i = 0; i < 6; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW);
  }

  pinMode(pinBuzzer, OUTPUT);

  // Configure push buttons with internal pull-ups (PRESS = LOW)
  pinMode(btnMode, INPUT_PULLUP);
  pinMode(btnMin, INPUT_PULLUP);
  pinMode(btnHrs, INPUT_PULLUP);
  pinMode(btnAlarm, INPUT_PULLUP);
}

// K-Map Minimized Next-State Function for BCD Modulo-10 Counter (0 to 9)
// Boolean Logic: Anext = !A | Bnext = B!A + !D!BA | Cnext = C!A + C!B + !CBA | Dnext = D!A + CBA
int kmapMod10Next(int val) {
  bool A = val & 0x01;
  bool B = (val >> 1) & 0x01;
  bool C = (val >> 2) & 0x01;
  bool D = (val >> 3) & 0x01;

  bool Anext = !A;
  bool Bnext = (B && !A) || (!D && !B && A);
  bool Cnext = (C && !A) || (C && !B) || (!C && B && A);
  bool Dnext = (D && !A) || (C && B && A);

  return (Dnext << 3) | (Cnext << 2) | (Bnext << 1) | Anext;
}

// K-Map Minimized Next-State Function for Modulo-6 Counter (0 to 5)
// Boolean Logic: Xnext = !X | Ynext = Y!X + !Z!YX | Znext = Z!X + YX
int kmapMod6Next(int val) {
  bool X = val & 0x01;
  bool Y = (val >> 1) & 0x01;
  bool Z = (val >> 2) & 0x01;

  bool Xnext = !X;
  bool Ynext = (Y && !X) || (!Z && !Y && X);
  bool Znext = (Z && !X) || (Y && X);

  return (Znext << 2) | (Ynext << 1) | Xnext;
}

// K-Map Minimized 12-Hour State Transition Engine (12 -> 01 transition)
void incrementHoursKMap(int &hTens, int &hUnits) {
  if (hTens == 1 && hUnits == 2) {
    hTens = 0;
    hUnits = 1;
  } else if (hTens == 0 && hUnits == 9) {
    hTens = 1;
    hUnits = 0;
  } else {
    hUnits = kmapMod10Next(hUnits);
  }
}

// K-Map Minimized Minute Transition Engine (00 -> 59)
void incrementMinutesKMap(int &mTens, int &mUnits) {
  if (mUnits == 9) {
    mUnits = 0;
    if (mTens == 5) {
      mTens = 0;
    } else {
      mTens = kmapMod6Next(mTens);
    }
  } else {
    mUnits = kmapMod10Next(mUnits);
  }
}

// Time Advance Logic driven by K-Map Functions
void advanceTimeKMap() {
  int nextSecUnits = kmapMod10Next(secUnits);
  
  if (secUnits == 9) {
    secUnits = 0;
    int nextSecTens = kmapMod6Next(secTens);
    
    if (secTens == 5) {
      secTens = 0;
      
      int prevMinUnits = minUnits;
      int prevMinTens  = minTens;
      incrementMinutesKMap(minTens, minUnits);
      
      if (prevMinTens == 5 && prevMinUnits == 9 && minUnits == 0 && minTens == 0) {
        incrementHoursKMap(hrsTens, hrsUnits);
      }
    } else {
      secTens = nextSecTens;
    }
  } else {
    secUnits = nextSecUnits;
  }

  // Trigger Alarm Match Check
  if (alarmEnabled && hrsTens == alarmHrsTens && hrsUnits == alarmHrsUnits &&
      minTens == alarmMinTens && minUnits == alarmMinUnits && secTens == 0 && secUnits == 0) {
    alarmRinging = true;
  }
}

void beep(int duration) {
  tone(pinBuzzer, 1000);
  delay(duration);
  noTone(pinBuzzer);
}

void handleButtons() {
  unsigned long currentMillis = millis();

  // Button 1: Mode Switch (A1)
  if (digitalRead(btnMode) == LOW) {
    if (currentMillis - lastDebounceMode > debounceDelay) {
      currentMode = (currentMode + 1) % 3;
      lastDebounceMode = currentMillis;
      beep(100);
    }
  }

  // Button 2: Minutes Increment (A2)
  if (digitalRead(btnMin) == LOW) {
    if (currentMillis - lastDebounceMin > debounceDelay) {
      if (currentMode == 1) {
        incrementMinutesKMap(minTens, minUnits);
        secUnits = 0;
        secTens  = 0;
      } else if (currentMode == 2) {
        incrementMinutesKMap(alarmMinTens, alarmMinUnits);
      }
      lastDebounceMin = currentMillis;
      beep(50);
    }
  }

  // Button 3: Hours Increment (A3)
  if (digitalRead(btnHrs) == LOW) {
    if (currentMillis - lastDebounceHrs > debounceDelay) {
      if (currentMode == 1) {
        incrementHoursKMap(hrsTens, hrsUnits);
      } else if (currentMode == 2) {
        incrementHoursKMap(alarmHrsTens, alarmHrsUnits);
      }
      lastDebounceHrs = currentMillis;
      beep(50);
    }
  }

  // Button 4: Alarm Toggle / Stop (A4)
  if (digitalRead(btnAlarm) == LOW) {
    if (currentMillis - lastDebounceAlarm > debounceDelay) {
      if (alarmRinging) {
        alarmRinging = false;
        noTone(pinBuzzer);
      } else {
        alarmEnabled = !alarmEnabled;
        beep(100);
        if (alarmEnabled) {
          delay(100);
          beep(100);
        }
      }
      lastDebounceAlarm = currentMillis;
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis += 1000;
    advanceTimeKMap();
  }

  // Sound Buzzer Pattern when Alarm Triggers
  if (alarmRinging) {
    if ((millis() / 250) % 2 == 0) {
      tone(pinBuzzer, 2000);
    } else {
      noTone(pinBuzzer);
    }
  }

  handleButtons();
  refreshDisplay();
}

void writeBCDAndBypasses(int val) {
  // Output BCD value to 7447 IC pins (A, B, C, D)
  digitalWrite(pinA, (val & 0x01) ? HIGH : LOW);
  digitalWrite(pinB, (val & 0x02) ? HIGH : LOW);
  digitalWrite(pinC, (val & 0x04) ? HIGH : LOW);
  digitalWrite(pinD, (val & 0x08) ? HIGH : LOW);

  // Output manual segment G line only
  digitalWrite(pinSegG, segGState[val]);
}

void refreshDisplay() {
  int digits[6];

  if (currentMode == 0 || currentMode == 1) {
    // Clock Mode: Display real-time clock (HH:MM:SS)
    digits[0] = hrsTens;
    digits[1] = hrsUnits;
    digits[2] = minTens;
    digits[3] = minUnits;
    digits[4] = secTens;
    digits[5] = secUnits;
  } else if (currentMode == 2) {
    // Alarm Mode: Display set alarm time (HH:MM:00)
    digits[0] = alarmHrsTens;
    digits[1] = alarmHrsUnits;
    digits[2] = alarmMinTens;
    digits[3] = alarmMinUnits;
    digits[4] = 0;
    digits[5] = 0;
  }

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      digitalWrite(digitPins[j], LOW);
    }

    writeBCDAndBypasses(digits[i]);

    digitalWrite(digitPins[i], HIGH);
    delayMicroseconds(1000);
    digitalWrite(digitPins[i], LOW);
  }
}
