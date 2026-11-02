#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

// LEFT MOTOR PINS
#define ENA 27
#define IN1 32
#define IN2 33

// RIGHT MOTOR PINS
#define ENB 13
#define IN3 25
#define IN4 26

// SPEED PRESETS
#define SPEED_SLOW   120
#define SPEED_NORMAL 200
#define SPEED_TURBO  240

int currentMaxSpeed = SPEED_NORMAL;

// MOTOR SPEED VARIABLES
int targetLeftSpeed = 0;
int targetRightSpeed = 0;

int currentLeftSpeed = 0;
int currentRightSpeed = 0;

// SAFETY TIMER
unsigned long lastCommandTime = 0;
const unsigned long TIMEOUT_MS = 500;


// SETUP
void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  Dabble.begin("UGV_ESP32");

  lastCommandTime = millis();

  Serial.println("UGV READY!");
}


// MOTOR CONTROL
void applyMotorPins(int leftSpd, int rightSpd) {
  // LEFT MOTOR
  if (leftSpd > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }
  else if (leftSpd < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
  else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  analogWrite(ENA, abs(leftSpd));

  // RIGHT MOTOR
  if (rightSpd > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  else if (rightSpd < 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  analogWrite(ENB, abs(rightSpd));
}


// STOP MOTORS
void stopMotors() {
  targetLeftSpeed = 0;
  targetRightSpeed = 0;

  currentLeftSpeed = 0;
  currentRightSpeed = 0;

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// SMOOTH SPEED RAMPING
void updateMotorRamping() {
  // LEFT MOTOR RAMPING
  if (currentLeftSpeed < targetLeftSpeed) {
    currentLeftSpeed = min(currentLeftSpeed + 15, targetLeftSpeed);
  }
  else if (currentLeftSpeed > targetLeftSpeed) {
    currentLeftSpeed = max(currentLeftSpeed - 15, targetLeftSpeed);
  }

  // RIGHT MOTOR RAMPING
  if (currentRightSpeed < targetRightSpeed) {
    currentRightSpeed = min(currentRightSpeed + 15, targetRightSpeed);
  }
  else if (currentRightSpeed > targetRightSpeed) {
    currentRightSpeed = max(currentRightSpeed - 15, targetRightSpeed);
  }

  applyMotorPins(currentLeftSpeed, currentRightSpeed);
}


// MAIN LOOP
void loop() {
  Dabble.processInput();

  bool inputReceived = false;

  // 1. SPEED SELECTION
  if (GamePad.isSelectPressed()) {
    currentMaxSpeed = SPEED_SLOW;
  }
  else if (GamePad.isTrianglePressed()) {
    currentMaxSpeed = SPEED_NORMAL;
  }
  else if (GamePad.isStartPressed()) {
    currentMaxSpeed = SPEED_TURBO;
  }

  // 2. DIRECTION CONTROL
  if (GamePad.isUpPressed()) {
    targetLeftSpeed = currentMaxSpeed;
    targetRightSpeed = currentMaxSpeed;
    inputReceived = true;
  }
  else if (GamePad.isDownPressed()) {
    targetLeftSpeed = -currentMaxSpeed;
    targetRightSpeed = -currentMaxSpeed;
    inputReceived = true;
  }
  else if (GamePad.isLeftPressed()) {
    targetLeftSpeed = -currentMaxSpeed;
    targetRightSpeed = currentMaxSpeed;
    inputReceived = true;
  }
  else if (GamePad.isRightPressed()) {
    targetLeftSpeed = currentMaxSpeed;
    targetRightSpeed = -currentMaxSpeed;
    inputReceived = true;
  }
  else {
    targetLeftSpeed = 0;
    targetRightSpeed = 0;
  }

  // 3. UPDATE COMMAND TIMER
  if (inputReceived) {
    lastCommandTime = millis();
  }

  // 4. SAFETY TIMEOUT
  if (millis() - lastCommandTime > TIMEOUT_MS) {
    targetLeftSpeed = 0;
    targetRightSpeed = 0;
  }

  // 5. UPDATE MOTOR SPEED
  updateMotorRamping();

  delay(10);
}
