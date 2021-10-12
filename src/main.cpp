#include <Arduino.h>

#include "joystick.h"
#include "stepper.h"
#include "parser.h"
#include "plotter.h"

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

void setup() {
  Serial.begin(9600);
  setMotorState(true);
  delay(5000);

  // Parse example SVG
  const std::string example =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path d=\"M 100 200 L 200 300\">\n"
    "</svg>";
  SVG svg = SVG(example);
  plotter.executeSVG(svg);

  // Disconnect the pins from the PWM signal
  ledcDetachPin(plotter.stepper1.stepPin);
  ledcDetachPin(plotter.stepper2.stepPin);
}

void loop() {
  // Fall back to joystick control
  joystick();
}