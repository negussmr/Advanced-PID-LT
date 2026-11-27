#include "Planner.h"
#include "Plan.h"

const int muxSIG = A0;
const int muxS0  = A4;
const int muxS1  = A3;
const int muxS2  = A2;
const int muxS3  = A1;

const int buttonPin = 11;
const int buzzerPin = 6;

const int AIN1 = 7;
const int AIN2 = 8;
const int PWMA = 9;

const int BIN1 = 5;
const int BIN2 = 4;
const int PWMB = 10;

int sensorValues[16];
int blackValues[16];
int whiteValues[16];
bool sensorValid[16];

bool running = false;

float Kp = 0.6;
float Ki = 0.000001;
float Kd = 30.0;

long lastError = 0;
float integral = 0;

const int defaultSpeed = 250;
const float slowFactor = 0.004;

const unsigned long loopPeriodUs = 2000;
unsigned long lastLoopUs = 0;

const int noiseGate = 150;
const long minTotal = 300;

const unsigned long lostConfirmMs = 20;
const unsigned long gapTimeMs = 250;
const unsigned long searchTimeoutMs = 2000;
const long gapError = 2000;
const int gapSpeed = 150;
const int searchSpeed = 220;
unsigned long lostSince = 0;
bool wasLost = true;

const int minSeparation = 50;
const int minValidSensors = 8;
const int calSamples = 200;

const bool debugPrint = false;

void playStartupSong() {
  tone(buzzerPin, 523, 100);
  delay(120);
  tone(buzzerPin, 659, 100);
  delay(120);
  tone(buzzerPin, 784, 150);
  delay(170);
  tone(buzzerPin, 1047, 250);
  delay(280);
  noTone(buzzerPin);
}

void playTapSong(bool starting) {
  if (starting) {
    tone(buzzerPin, 523, 80);
    delay(100);
    tone(buzzerPin, 659, 80);
    delay(100);
    tone(buzzerPin, 784, 120);
    delay(140);
    tone(buzzerPin, 1047, 180);
    delay(200);
  } else {
    tone(buzzerPin, 784, 80);
    delay(100);
    tone(buzzerPin, 659, 80);
    delay(100);
    tone(buzzerPin, 523, 150);
    delay(170);
  }
  noTone(buzzerPin);
}

void beep(int times) {
  for (int i = 0; i < times; i++) {
    tone(buzzerPin, 1000, 100);
    delay(180);
  }
  noTone(buzzerPin);
}

void errorBeep() {
  tone(buzzerPin, 200, 700);
  delay(800);
  noTone(buzzerPin);
}

void waitForTap() {
  while (digitalRead(buttonPin) == LOW) {}
  delay(50);
  while (digitalRead(buttonPin) == HIGH) {}
  delay(50);
  while (digitalRead(buttonPin) == LOW) {}
  delay(50);
}

void selectMuxChannel(byte channel) {
  digitalWrite(muxS0, bitRead(channel, 0));
  digitalWrite(muxS1, bitRead(channel, 1));
  digitalWrite(muxS2, bitRead(channel, 2));
  digitalWrite(muxS3, bitRead(channel, 3));
  delayMicroseconds(5);
}

void readAllSensors() {
  for (byte ch = 0; ch < 16; ch++) {
    selectMuxChannel(ch);
    analogRead(muxSIG);
    sensorValues[ch] = analogRead(muxSIG);
  }
}

void printSensors() {
  for (byte ch = 0; ch < 16; ch++) {
    Serial.print(sensorValues[ch]);
    Serial.print('\t');
  }
  Serial.println();
}

void sampleSensors(int *target) {
  long sums[16];
  for (byte ch = 0; ch < 16; ch++) sums[ch] = 0;

  for (int s = 0; s < calSamples; s++) {
    readAllSensors();
    for (byte ch = 0; ch < 16; ch++) sums[ch] += sensorValues[ch];
  }

  for (byte ch = 0; ch < 16; ch++) target[ch] = sums[ch] / calSamples;
}

void calibrate() {
  bool ok = false;

  while (!ok) {
    waitForTap();
    delay(300);
    tone(buzzerPin, 1000, 250);
    sampleSensors(blackValues);
    delay(300);
    beep(3);

    waitForTap();
    delay(300);
    tone(buzzerPin, 1000, 250);
    sampleSensors(whiteValues);
    delay(300);
    beep(4);

    int validCount = 0;
    for (byte ch = 0; ch < 16; ch++) {
      sensorValid[ch] = abs(blackValues[ch] - whiteValues[ch]) >= minSeparation;
      if (sensorValid[ch]) validCount++;
    }

    ok = validCount >= minValidSensors;
    if (!ok) errorBeep();
  }
}

int normalized(byte ch) {
  if (!sensorValid[ch]) return 0;

  long n = ((long)sensorValues[ch] - whiteValues[ch]) * 1000L /
           (blackValues[ch] - whiteValues[ch]);

  return (int)constrain(n, 0, 1000);
}

void setMotorA(int speed) {
  speed = constrain(speed, 0, 255);

  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  analogWrite(PWMA, speed);
}

void setMotorB(int speed) {
  speed = constrain(speed, 0, 255);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMB, speed);
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}

void setMotorsPair(int left, int right) {
  setMotorA(left);
  setMotorB(right);
}

Planner planner(plan, sizeof(plan) / sizeof(plan[0]), setMotorsPair, defaultSpeed);

int computePID(long error) {
  integral += error;
  integral = constrain(integral, -20000, 20000);

  long derivative = error - lastError;
  lastError = error;

  float output = (Kp * error) + (Ki * integral) + (Kd * derivative);

  return (int)constrain(output, -255, 255);
}

void searchForLine(unsigned long lostFor) {
  if (lostFor > searchTimeoutMs) {
    stopMotors();
    return;
  }

  if (abs(lastError) < gapError && lostFor < gapTimeMs) {
    setMotorA(gapSpeed);
    setMotorB(gapSpeed);
    return;
  }

  if (lastError < 0) {
    setMotorA(searchSpeed);
    setMotorB(0);
  } else {
    setMotorA(0);
    setMotorB(searchSpeed);
  }
}

void followLine() {
  long weightedSum = 0;
  long total = 0;

  for (byte ch = 0; ch < 16; ch++) {
    int n = normalized(ch);
    if (n < noiseGate) continue;

    weightedSum += (long)ch * 1000L * n;
    total += n;
  }

  if (total < minTotal) {
    unsigned long now = millis();
    if (lostSince == 0) lostSince = now;

    unsigned long lostFor = now - lostSince;
    wasLost = true;

    if (lostFor < lostConfirmMs) return;

    searchForLine(lostFor);
    integral = 0;
    return;
  }

  lostSince = 0;

  long error = (weightedSum / total) - 7500;

  if (wasLost) {
    lastError = error;
    wasLost = false;
  }

  int turn = computePID(error);
  int speed = planner.baseSpeed() - (int)(abs(error) * slowFactor);

  setMotorA(speed - turn);
  setMotorB(speed + turn);
}

void handleButton() {
  static bool lastState = HIGH;
  bool state = digitalRead(buttonPin);

  if (lastState == HIGH && state == LOW) {
    running = !running;

    stopMotors();

    if (running) {
      integral = 0;
      lastError = 0;
      lostSince = 0;
      wasLost = true;
      planner.reset();
    }

    playTapSong(running);
    delay(200);

    lastLoopUs = micros();
  }

  lastState = state;
}

void setup() {
  Serial.begin(115200);

  ADCSRA = (ADCSRA & 0xF8) | 0x05;
  TCCR1B = (TCCR1B & 0xF8) | 0x02;

  pinMode(muxS0, OUTPUT);
  pinMode(muxS1, OUTPUT);
  pinMode(muxS2, OUTPUT);
  pinMode(muxS3, OUTPUT);
  pinMode(muxSIG, INPUT);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  stopMotors();
  digitalWrite(buzzerPin, LOW);

  delay(100);
  playStartupSong();

  calibrate();

  running = false;
}

void loop() {
  handleButton();

  if (!running) {
    stopMotors();
    return;
  }

  unsigned long now = micros();
  if (now - lastLoopUs < loopPeriodUs) return;
  lastLoopUs = now;

  readAllSensors();

  int active = 0;
  for (byte ch = 0; ch < 16; ch++) {
    if (normalized(ch) >= 500) active++;
  }

  bool overriding = planner.update(active);

  if (planner.stopRequested()) {
    running = false;
    stopMotors();
    return;
  }

  if (overriding) return;

  if (planner.consumeStepDone()) {
    integral = 0;
    wasLost = true;
  }

  if (debugPrint) printSensors();
  followLine();
}
