#include <Arduino.h>
#include "plotter.h"

// TODO: Change accuracy based on curve length

// Implementation of De Casteljau's algorithm
// parametric function with `t` -> move t from 0 to 1

// `B(t) = (1 - t)^2 P_0 + 2t (1 - t) P_1 + t^2 P_2`
int Plotter::bezierQuadratic(Point p1, Point p2) {
  // `accuracy` defines the amount of steps between
  float x, y;
  float increase = 1.0/20.0;
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

// `B(t) = (1 - t)^3 P_0 + 3t (1 - t)^2 P_1 + 3t^2 (1 - t) P_2 + t^3 P_3`
int Plotter::bezierCubic(Point p1, Point p2, Point p3) {
  // `accuracy` defines the amount of steps between
  float x, y;
  float increase = 1.0/20.0;
  for (float t = increase; t < 1.0 + increase; t += increase) {
    x = pow((1.0 - t), 3) * this->pos.x + 3.0 * t * pow((1.0 - t), 2) * p1.x + 3.0 * pow(t, 2) * (1.0 - t) * p2.x + pow(t, 3) * p3.x;
    y = pow((1.0 - t), 3) * this->pos.y + 3.0 * t * pow((1.0 - t), 2) * p1.y + 3.0 * pow(t, 2) * (1.0 - t) * p2.y + pow(t, 3) * p3.y;
    if (this->moveTo(Point{x, y}) != 0) {
      Serial.println("Failed to move!");
    }
    delay(100);
  }
  return 0;
}