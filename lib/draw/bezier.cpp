#include <Arduino.h>
#include "draw.h"

// Implementation of De Casteljau's algorithm
// parametric function with `t` -> move t from 0 to 1

// `B(t) = (1 - t)^2 P_0 + 2t (1 - t) P_1 + t^2 P_2`
int Plotter::bezierQuadratic(Point p1, Point p2) {
  // `accuracy` defines the amount of steps between
  float x, y;
  float increase = 1.0/10.0;
  // Move parameter t from 0.0 to 1.0
  for (float t = increase; t < 1.0 + increase; t += increase) {
    x = pow((1.0 - t), 2) * this->pos.x + 2.0 * t * (1.0 - t) * p1.x + pow(t, 2) * p2.x;
    y = pow((1.0 - t), 2) * this->pos.y + 2.0 * t * (1.0 - t) * p1.y + pow(t, 2) * p2.y;
    if (this->moveTo(Point{x, y}) != 0) {
      Serial.println("Failed to move!");
    }
    delay(100);
  }
  return 0;
}