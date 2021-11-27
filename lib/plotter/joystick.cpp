// Code to use plotter with joystick
// Parts from Ivo Blöchliger - KSBG

#include <Arduino.h>
#include "stepper.h"
#include "plotter.h"

const int joyPins[3] = { 34, 35, 26 };

// Move the plotter with the connected joystick
void Plotter::joystick(bool draw) {
  // Setup all the pins
  pinMode(joyPins[0], INPUT);
  pinMode(joyPins[1], INPUT);
  pinMode(joyPins[2], INPUT_PULLUP);
  // Detach pins
  this->stepper1.detachPin();
  this->stepper2.detachPin();

  long int lastMove = 0;
  long int maxdelay = 5000;
  long int stepdelay = maxdelay;
  int stepnum = 1;

  int steps[2] = { 0, 0 };

  while (true) {
    int a[2];
    a[0] = analogRead(joyPins[0]);
    a[1] = analogRead(joyPins[1]);
    int sw = digitalRead(joyPins[2]);
  
    int vx=(a[1]<500 ? -1 : (a[1]>3500 ? 1 : 0));
    int vy=(a[0]<500 ? -1 : (a[0]>3500 ? 1 : 0));

    int m0 = vx-vy;
    int m1 = -vx-vy;
    if (m0!=0 || m1!=0) {
      if (millis()-lastMove<2) {
        if (stepdelay>100) {
          stepdelay = maxdelay*(sqrt(stepnum+1)-sqrt(stepnum));
          stepnum++;
        }
      } else {
        stepdelay=maxdelay;
        stepnum = 1;
      }
      delayMicroseconds(stepdelay);
      lastMove=millis();
    }
    this->stepper1.applyDirection(m0 < 0);
    this->stepper2.applyDirection(m1 < 0);
  
    if (m0!=0) {
      this->stepper1.step();
      if (m0 < 0) {
        steps[0]--;
      } else {
        steps[0]++;
      }
    }
    if (m1!=0) {
      this->stepper2.step();
      if (m1 < 0) {
        steps[1]--;
      } else {
        steps[1]++;
      }
    }
    if (!sw) {
      if (draw) {
        this->pen.toggle();
      } else {
        break;
      }
    }
  }

  // Calculate the string lengths
  this->strings[0] = this->strings[0] + (float)steps[0] * perstep;
  this->strings[1] = this->strings[1] + (float)steps[1] * perstep;

  // Calculate the coordinates
  // a^2 + b^2 = c_1^2; a_2^2 + b^2 = c_2^2
  float a, b;
  a = (pow(this->strings[0], 2) - pow(this->strings[1], 2) + pow(boardWidth, 2)) / (2.0 * boardWidth);
  b = sqrt(pow(this->strings[0], 2) - pow(a, 2));
  this->pos.x = a - boardWidth / 2.0;
  this->pos.y = boardHeight - b;

  // Attach pins again
  this->stepper1.attachPin();
  this->stepper2.attachPin();
}
