#include <Arduino.h>
#include <unity.h>

#include "stepper.h"
#include "plotter.h"

StepperMotor stepper1 = StepperMotor(0, dirPins[0], stepPins[0]);
StepperMotor stepper2 = StepperMotor(1, dirPins[1], stepPins[1]);

Point start = Point(0, 0);
float *startStringLength = start.calculatePosition();

Servo pen = Servo(penPin);

Plotter plotter = {
  .pos = start, 
  // NOTE: Better way?
  .stringLength = { startStringLength[0], startStringLength[1] },
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

// SVG Files

void test_draw_svg_square() {
  const std::string square = 
    "<svg width=\"200\" height=\"200\" viewBox=\"0 0 200 200\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">"
      "<path d=\"M 50 50 H 150 V 150 H 50 Z\" stroke=\"black\"/>"
    "</svg>";
  SVG svg = SVG(square);
  plotter.executeSVG(svg);
}

void test_draw_svg_heart() {
  const std::string heart = 
    "<svg width=\"200\" height=\"200\" viewBox=\"0 0 200 200\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">"
      "<path d=\"M100 58.0145C130.185 97.341 172.444 166.995 100 131.002\" stroke=\"black\"/>"
      "<path d=\"M99.5341 58C69.7007 97.3333 27.9341 167 99.5341 131\" stroke=\"black\"/>"
    "</svg>";
  SVG svg = SVG(heart);
  plotter.executeSVG(svg);
}

// Run tests on esp32dev 
// `pio test -e esp32dev -v`
void setup() {
  Serial.begin(9600);
  setMotorState(true);
  delay(2000);

  UNITY_BEGIN();

  // RUN_TEST(test_draw_line_square);
  // RUN_TEST(test_draw_line_triangle);
  // RUN_TEST(test_draw_bezier_quadratic);
  // RUN_TEST(test_draw_bezier_cubic);

  // RUN_TEST(test_draw_svg_square);
  RUN_TEST(test_draw_svg_heart);

  // Return to start
  plotter.pen.penUp();
  plotter.moveTo(start);

  UNITY_END();
}

void loop() {}
