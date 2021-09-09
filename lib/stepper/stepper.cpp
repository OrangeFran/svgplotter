#include <Arduino.h>
#include "stepper.h"

float perstep = 0.019625; // 0.07925;

// Pins to move and set direction (left, right)
int stepPins[2] = {17, 19};
int dirPins[2] = {16, 18};

// Pins to control state of motors
int enPin = 12;
int resPin = 33;
int sleepPin = 32;

// Global turned on state of motors
bool motorState = false;

// Turn the motors on
void setMotorState(bool on) {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, !on);
  pinMode(resPin, OUTPUT);
  digitalWrite(resPin, on);
  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, on);
  motorState = on;
}

// Old function still needed for lib/joystick
// Do one step with the specified motor
void step(int stepper) {
  digitalWrite(stepPins[stepper], HIGH);
  delayMicroseconds(2);
  digitalWrite(stepPins[stepper], LOW);
}

// Constructor
StepperMotor::StepperMotor(int index, int dirPin, int stepPin) {
  this->index = index;
  this->dirPin = dirPin;
  this->stepPin = stepPin;
}

void StepperMotor::setVelocity(float newVelocity) {
  velocity = newVelocity;
}

// Do one step
void StepperMotor::step() {
  digitalWrite(this->stepPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(this->stepPin, LOW);
}

// Make the string for a certain motor longer/shorter
int StepperMotor::travel(int distance) {
  int stepper = this->index;
  float velocity = this->velocity;

  // Set the direction
  digitalWrite(this->dirPin, distance > 0 ? stepper : (int)!(bool)stepper);

  // Calculate and do steps
  // The direction is already set, so the prefix can be
  // removed with abs()
  int steps = abs(distance)/perstep;
  TickType_t x_last_wake_time = xTaskGetTickCount();
  for (int i = 0; i < steps; i++) {
    this->step();
    // delayMicroseconds will not work,
    // because it does not provide time for the watchdog
    // vTaskDelay is absolute, vTaskDelayUntil is relative
    vTaskDelayUntil(&x_last_wake_time, velocity * portTICK_PERIOD_MS);
    // vTaskDelay(velocity * portTICK_PERIOD_MS);
  }

  return 0;
}
