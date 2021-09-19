#include <Arduino.h>
#include "stepper.h"

// #define CONFIG_FREERTOS_HZ 1000

// TODO: measure/calculate exact mm per step
float perstep = 0.009817477;
                // 0.019625;
                // 0.07925;

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
  // Pins
  this->index = index; // 0 / 1
  this->dirPin = dirPin;
  this->stepPin = stepPin;

  // Set the direction pin to OUTPUT
  pinMode(this->dirPin, OUTPUT);
  pinMode(this->stepPin, OUTPUT);

  // Attach a channel to the stepping pin 
  // There are only 8 timers for 16 channels
  // -> subsequent channels use the same timer
  // and thus cannot be driven by different frequencies
  ledcAttachPin(this->stepPin, this->index * 2);
  // Frequency is arbitrary on init and will be set
  // with the velocity
  ledcSetup(this->index * 2, 1000, 8);
}

void StepperMotor::setVelocity(float velocity) {
  // Velocity (delay in µs between steps) and freq in Hz
  this->velocity = velocity;
  this->frequency = (float)1000000/this->velocity;

  // Set the direction
  digitalWrite(this->dirPin, velocity > 0 ? this->index : (int)!(bool)this->index);
  // Velocity specifies the delay in microseconds
  ledcWriteTone(this->index * 2, (int)this->frequency);
}

// Do one step
void StepperMotor::step() {
  digitalWrite(this->stepPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(this->stepPin, LOW);
}

// Make the string for a certain motor longer/shorter
int StepperMotor::start() {
  int stepper = this->index;
  float velocity = this->velocity;

  // // Calculate and do steps
  // // The direction is already set, so the prefix can be
  // // removed with abs()
  // int steps = abs(distance)/perstep;

  // With current frequency: One tick = one millisecond
  // TickType_t x_last_wake_time = xTaskGetTickCount();
  // TickType_t frequency = velocity * portTICK_PERIOD_MS;
  // for (int i = 0; i < steps; i++) {
  //   this->step();
  //   // delayMicroseconds will not work,
  //   // because it does not provide time for the watchdog
  //   // vTaskDelay is absolute, vTaskDelayUntil is relative
  //   vTaskDelayUntil(&x_last_wake_time, frequency);
  // }

  // The dutyCycle does not have to be that accurate
  ledcWrite(stepper * 2, round(2/velocity * 255));
  return 0;
}

// Stop the motor (= set dutyCycle to 0)
int StepperMotor::stop() {
  ledcWrite(this->index * 2, 0);
}