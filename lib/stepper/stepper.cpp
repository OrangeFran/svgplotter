#include <Arduino.h>
#include "stepper.h"

// #define CONFIG_FREERTOS_HZ 1000

// TODO: measure/calculate exact mm per step
// Current number based on trial and error
float perstep = 0.01;
                // 0.009817477;
                // 0.019625;
                // 0.07925;

// Pins to set direction and move: { leftPin, rightPin }
int dirPins[2] = {16, 18};
int stepPins[2] = {17, 19};

// Pins to control state of motors
int enPin = 12;
int resPin = 33;
int sleepPin = 32;

// Global turned on state of motors
bool motorState = false;

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
  ledcSetup(this->index * 2, 1000, 14);
}

void StepperMotor::setVelocity(int velocity, bool shorter) {
  // Velocity in sps
  this->velocity = velocity;

  // Set the direction
  digitalWrite(this->dirPin, shorter ? (int)!(bool)this->index : this->index);
  // Velocity specifies the delay in microseconds
  ledcWriteTone(this->index * 2, (int)this->velocity);

  // Change the resolution
  // This is necessary because the available resolution
  // reduces if the frequency increases
  //
  // Max resolution possible for f: f(f) = log_2(80MHz/f)
  //   for f = 1 -> ~26
  //
  // Formulae:
  //   (f/500000) * (2**x - 1) = 1
  //   => x = 1.4427 * ln(1 + 500000/f)
  //
  // Delay of 2 microseconds = freq of 500000Hz
  // int resolution = round(1.4427 * log(1.0 + (500000.0/this->velocity)));
}

// Do one step
void StepperMotor::step() {
  digitalWrite(this->stepPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(this->stepPin, LOW);
}

// Make the string for a certain motor longer/shorter
int StepperMotor::start() {
  // // Calculate and do steps
  // // The direction is already set, so the prefix can be
  // // removed with abs()
  // int steps = abs(distance)/perstep;

  // // With current frequency: One tick = one millisecond
  // // TickType_t x_last_wake_time = xTaskGetTickCount();
  // // TickType_t frequency = velocity * portTICK_PERIOD_MS;
  // for (int i = 0; i < steps; i++) {
  //   this->step();
  //   // delayMicroseconds will not work,
  //   // because it does not provide time for the watchdog
  //   // vTaskDelay is absolute, vTaskDelayUntil is relative
  //   vTaskDelayUntil(&x_last_wake_time, frequency);
  // }

  // The dutyCycle does not have to be accurate to the point
  // Delay of 2 microseconds = freq of 500000Hz
  int dutyCycle = round((float)this->velocity/500000.0 * 16383.0);
  // Use one if dutyCycle is too small
  ledcWrite(this->index * 2, dutyCycle == 0 ? 1 : dutyCycle);
  return 0;
}

// Stop the motor (= set dutyCycle to 0)
int StepperMotor::stop() {
  ledcWrite(this->index * 2, 0);
}