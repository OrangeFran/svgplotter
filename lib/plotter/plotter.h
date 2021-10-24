#ifndef PLOTTER_H
#define PLOTTER_H

#include "parser.h"
#include "stepper.h"
#include "servo.h"

// Whiteboard
extern const float boardWidth;
extern const float boardHeight;

extern const float baseVelocity;
extern const int joyPins[3];

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
    StepperMotor stepper1; // Left
    StepperMotor stepper2; // Right
    Servo pen;

    int moveTo(Point);
    int bezierLinear(Point);
    int bezierQuadratic(Point, Point);
    int bezierCubic(Point, Point, Point);

    int executeSVG(SVG);
    void joystick(void);
};

#endif