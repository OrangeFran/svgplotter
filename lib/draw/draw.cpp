#include <Arduino.h>
#include "draw.h"

// Whiteboard
const float boardWidth = 1930.0;    // mm
const float boardHeight = 1170.0;   // mm
// Delay of 500 µs in rps: 2000Hz
const float baseVelocity = 2000.0;  // Hz

Point::Point(float x, float y) {
  this->x = x;
  this->y = y;
}

// Calculate the length of the string at
// the point (x, y) with the law of Pythagoras
float *Point::calculatePosition() {
  float newS1 = sqrt(pow(boardWidth/2 + this->x, 2) + pow(boardHeight - this->y, 2));
  float newS2 = sqrt(pow(boardWidth/2 - this->x, 2) + pow(boardHeight - this->y, 2));
  // Keyword `static` makes sure that the array
  // is not deleted (out of scope) after the return statement
  static float position[2] = { newS1, newS2 };
  return position;
}