#ifndef PLOTTER_H
#define PLOTTER_H

#include "parser.h"
#include "stepper.h"
#include "servo.h"

// Whiteboard
extern const float boardWidth;
extern const float boardHeight;
extern const float baseVelocity;

// Two-dimensional point
class Point {
  public:
    float x;
    float y;

    // Constructor
    Point(float, float);
    // Calculates the string length for the point
    float *getStrings(void);
};

// Position of the plotter
class Plotter {
  public:
    Point pos;
    float strings[2];
    StepperMotor stepper1; // left
    StepperMotor stepper2; // right
    Servo pen;

    // No constructor
    int moveTo(Point);
    int bezierLinear(Point);
    int bezierQuadratic(Point, Point);
    int bezierCubic(Point, Point, Point);
    int executeSVG(SVG);
};

#endif