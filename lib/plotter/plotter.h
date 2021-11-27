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
    // For faster access and fewer calculations
    float strings[2];
    StepperMotor stepper1; // Left
    StepperMotor stepper2; // Right
    Servo pen;

    // Make one point
    void makePoint();
    void moveTo(Point);
    // Split one line into multiple segements
    // and draw them individually
    void splitMove(Point);
    void bezierQuadratic(Point, Point);
    void bezierCubic(Point, Point, Point);

    // Execute svg
    void executeSVG(SVG);
    // Enable joystick control
    void joystick(bool);
};

#endif