#include <Arduino.h>

// Pins to set direction
// HIGH -> clockwise
// LOW -> counter clockwise
int dirPins[] = {16, 18};
// Pins to move
int stepPins[] = {17, 19};

// Pins to control state of motors
int enPin = 12;
int resPin = 33;
int sleepPin = 32;
// ON/OFF global state of motors
bool motorstate = false;

// Directions
// {motor1, motor2}
// 1 -> clockwise, 0 -> counterclockwise
int goUp[] = {1, 0};
int goDown[] = {0, 1};
int goLeft[] = {1, 1};
int goRight[] = {0, 0};

void motorState(bool on) {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, !on);
  pinMode(resPin, OUTPUT);
  digitalWrite(resPin, on);
  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, on);
  motorstate = on;
}

// Do one step with the specified motor
void step(int motor) {
  digitalWrite(stepPins[motor], HIGH);
  delayMicroseconds(2);
  digitalWrite(stepPins[motor], LOW);
}

void changeDirection(int direction[]) {
  for (int i = 0; i < 2; i++) {
    digitalWrite(dirPins[i], direction[i]);
  }
}

// TODO: Make it work!
// Travel a certain distance in mm 
void travel(int distance) {
  int steps = distance/0.019625;
  for (int i = 0; i < steps; i++)
    step(0); step(1); delay(5);
}

// Serial.begin(9600);
// Serial.println("Setup ...");

void setup() {
  // Set all pins to output
  for (int p = 0; p < 2; p++)
    pinMode(dirPins[p], OUTPUT);
  for (int p = 0; p < 2; p++)
    pinMode(stepPins[p], OUTPUT);
  // Set direction to going up
  changeDirection(goLeft);
  // Turn on the motors itself
  motorState(true);
  // travel(100);
}

void loop() {
  step(0);
  step(1);
  // Velocity for now ...
  delay(5);
}