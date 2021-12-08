#include <Arduino.h>
#include <unity.h>

#include "stepper.h"
#include "plotter.h"

StepperMotor stepper1 = StepperMotor(0, dirPins[0], stepPins[0]);
StepperMotor stepper2 = StepperMotor(1, dirPins[1], stepPins[1]);

Point start = Point(0, 0);
float *startStrings = start.getStrings();

Servo pen = Servo(penPin);

Plotter plotter = {
  .pos = start, 
  // NOTE: Better way?
  .strings = { startStrings[0], startStrings[1] },
  .stepper1 = stepper1,
  .stepper2 = stepper2,
  .pen = pen,
};

// Manual

void test_draw_line_square() {
  plotter.pen.penDown();
  plotter.moveTo(Point(0, 100));
  plotter.moveTo(Point(100, 100));
  plotter.moveTo(Point(100, 0));
  plotter.moveTo(Point(0, 0));
}

void test_draw_line_triangle() {
  plotter.pen.penDown();
  plotter.moveTo(Point(0, 100));
  plotter.moveTo(Point(100, 100));
  plotter.moveTo(Point(0, 0));
}

void test_draw_bezier_quadratic() {
  plotter.pen.penDown();
  plotter.bezierQuadratic(Point(0, 100), Point(100, 100));
}

void test_draw_bezier_cubic() {
  plotter.pen.penDown();
  plotter.bezierCubic(Point(0, 100), Point(100, 100), Point(100, 0));
}

// Run tests on esp32dev 
// `pio test -e esp32dev -v`
void setup() {
  Serial.begin(9600);
  setMotorState(true);
  delay(2000);

  // Make sure the pen is up
  plotter.makePoint();

  // Select position
  plotter.joystick(false);

  UNITY_BEGIN();

  // Simple
  RUN_TEST(test_draw_line_square);
  RUN_TEST(test_draw_line_triangle);
  RUN_TEST(test_draw_bezier_quadratic);
  plotter.pen.penUp();
  plotter.moveTo(Point(0, 0));
  RUN_TEST(test_draw_bezier_cubic);

  UNITY_END();

  // Return to start
  plotter.pen.penUp();
  plotter.moveTo(start);
  setMotorSleep(true);
}

void loop() {}
