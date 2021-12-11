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

    // Move directly to a point
    // Results in a curved line (workaround: `splitMove`)
    void moveTo(Point, bool = true);

    // Split one line into multiple segements
    // and draw them individually -> straight line
    void splitMove(Point);

    // Draw a quadratic and a cubic bezier curve
    // The first point will be the current position of the plotter
    void bezierQuadratic(Point, Point);
    void bezierCubic(Point, Point, Point);

    // Execute svg
    void executeSVG(SVG);

    // Control plotter by moving the joystick
    // `bool` specifies
    // -> if joystick push down
    // -> or stops the control or moves the pen
    // Plotter position updates before the function returns
    void joystick(bool);
};

#endif