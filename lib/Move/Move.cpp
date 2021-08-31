#include <Arduino.h>
#include "move.h"

// Pins to set direction
int dirPins[] = {16, 18}; // (left, right)
// Pins to move
int stepPins[] = {17, 19}; // (left, right)

// Pins to control state of motors
int enPin = 12;
int resPin = 33;
int sleepPin = 32;

// Do one step with the specified motor
void step(int stepper) {
  digitalWrite(stepPins[stepper], HIGH);
  delayMicroseconds(2);
  digitalWrite(stepPins[stepper], LOW);
}

// Global turned on state of motors
bool motorstate = false;

// Turn the motors on
void motorState(bool on) {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, !on);
  pinMode(resPin, OUTPUT);
  digitalWrite(resPin, on);
  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, on);
  motorstate = on;
}
