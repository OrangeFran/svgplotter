#include <Arduino.h>
#include "servo.h"

const int servoPin = 22;

const int ledcDownFreq = (int)(1.0/20.0 * 256.0);
const int ledcUpFreq = (int)(1.5/20.0 * 256.0);

Servo::Servo(int pin) {
  this->pin = pin;
  this->down = false;
  
  // Set up a simple pwm signal
  pinMode(this->pin, OUTPUT);
  ledcSetup(8, 50, 8);
  ledcAttachPin(this->pin, 8);
}

int Servo::penDown() {
  if (!this->down) {
    this->down = true;
    ledcWrite(8, ledcDownFreq);
    delay(200);
    ledcWrite(8, 0);
    delay(200);
  }
  return 0;
}

int Servo::penUp() {
  if (this->down) {
    this->down = false;
    ledcWrite(8, ledcUpFreq);
    delay(200);
    ledcWrite(8, 0);
    delay(200);
  }
  return 0;
}

int Servo::toggle() {
  if (this->down) {
    this->penUp();
  } else {
    this->penDown();
  }
  return 0;
}