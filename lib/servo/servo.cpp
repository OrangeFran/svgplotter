#include <Arduino.h>
#include "servo.h"

const int servoPin = 22;

Servo::Servo(int pin) {
  this->pin = pin;
  this->down = false;
  
  // Setup pwm signal
  pinMode(this->pin, OUTPUT);
  ledcSetup(8, 50, 8);
  ledcAttachPin(this->pin, 8);
}

int Servo::penDown() {
  if (!down) {
    this->down = true;
    ledcWrite(8, (int)(1.0/20.0 * 256.0));
    delay(200);
    ledcWrite(8, 0);
    delay(200);
  }
  return 0;
}

int Servo::penUp() {
  if (down) {
    this->down = false;
    ledcWrite(8, (int)(1.5/20.0 * 256.0));
    delay(200);
    ledcWrite(8, 0);
    delay(200);
  }
  return 0;
}

int Servo::toggle() {
  if (down) {
    this->penUp();
  } else {
    this->penDown();
  }
  return 0;
}