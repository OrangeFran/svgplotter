#include <Arduino.h>
#include <unity.h>
#include "stepper.h"
#include "draw.h"

StepperMotor stepper1 = StepperMotor(0, dirPins[0], stepPins[0]);
StepperMotor stepper2 = StepperMotor(1, dirPins[1], stepPins[1]);

Point start = Point(0, 0);
float *startStringLength = start.calculatePosition();

Plotter plotter = {
  .pos = start, 
  // NOTE: Better way?
  .stringLength = { startStringLength[0], startStringLength[1] },
  .stepper1 = stepper1,
  .stepper2 = stepper2,
};

void test_draw_line_square() {
  plotter.moveTo(Point(0, 100));
  plotter.moveTo(Point(100, 100));
  plotter.moveTo(Point(100, 0));
  plotter.moveTo(Point(0, 0));
}

void test_draw_line_triangle() {
  plotter.moveTo(Point(0, 100));
  plotter.moveTo(Point(100, 100));
  plotter.moveTo(Point(0, 0));
}

void test_draw_bezier_quadratic() {
  plotter.bezierQuadratic(Point(0, 100), Point(100, 100));
  plotter.moveTo(Point(0, 0));
}

void test_draw_bezier_cubic() {
  plotter.bezierCubic(Point(0, 100), Point(100, 100), Point(100, 0));
  plotter.moveTo(Point(0, 0));
}

// Run tests on esp32dev 
// `pio test -e esp32dev -v`
void setup() {
  delay(2000);
  UNITY_BEGIN();

  RUN_TEST(test_draw_line_square);
  RUN_TEST(test_draw_line_triangle);
  RUN_TEST(test_draw_bezier_quadratic);
  RUN_TEST(test_draw_bezier_cubic);

  UNITY_END();
}

void loop() {}