#include <Arduino.h>
#include <driver/ledc.h>
#include "stepper.h"
#include <string>

#define TIMER_I(i) (i == 0 ? LEDC_TIMER_0 : LEDC_TIMER_1)
#define CHANNEL_I(i) (i == 0 ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1)

// 20 * π / (200 * 32)
const float perstep = 0.009817477;
// Global turned on state of motors
bool motorState = false;

// Pins to control state of motors
const int enPin = 12;
const int resPin = 33;
const int sleepPin = 32;
const int penPin = 22;

// Pins to set direction and move
// { leftPin, rightPin }
const int dirPins[2] = { 16, 18 };
const int stepPins[2] = { 17, 19 };

// Turn the controlboard, motors on
void setMotorState(bool on) {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, !on);
  pinMode(resPin, OUTPUT);
  digitalWrite(resPin, on);
  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, on);
  motorState = on;
}

// Constructor
StepperMotor::StepperMotor(
  int index, int dirPin, int stepPin
) {
  this->index = index;
  this->dirPin = dirPin;
  this->stepPin = stepPin;

  // Set the pins to OUTPUT
  pinMode(this->dirPin, OUTPUT);
  pinMode(this->stepPin, OUTPUT);
}

// Do one step
void StepperMotor::step() {
  digitalWrite(this->stepPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(this->stepPin, LOW);
}

// Set the direction (counter-clockwise -> 0, clockwise -> 1)
void StepperMotor::setDirection(bool shorter) {
  digitalWrite(this->dirPin, shorter ? (int)!(bool)this->index : this->index);
}

void StepperMotor::setVelocity(int delay, bool shorter) {
  // Velocity in sps
  this->delay = delay;
  this->setDirection(shorter);
}

void doSteps(void *arg) {
  StepperMotor *stepper = (StepperMotor *)arg;
  int stepsDone = 0;
  while (stepsDone != stepper->_steps) {
    stepper->step();
    stepsDone += 1;
    delayMicroseconds(stepper->delay);
  }
  Serial.printf("Finished loop (%d)!\n", stepper->index);
}

// Make the string for a certain motor longer/shorter
void StepperMotor::start(int steps) {
  this->_steps = steps;
  char *name; sprintf(name, "Stepper Task %d", this->index);

  // Create the task on the dedicated core
  xTaskCreatePinnedToCore(
    doSteps,
    name,
    10000,
    this,
    1,
    this->task,
    this->index
  );

  Serial.printf("Finished function (%d)!\n", this->index);
}
