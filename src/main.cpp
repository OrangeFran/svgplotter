#include <Arduino.h>
#include <SD.h>

#include "stepper.h"
#include "parser.h"
#include "plotter.h"

StepperMotor stepper1 = StepperMotor(0, dirPins[0], stepPins[0]); // left
StepperMotor stepper2 = StepperMotor(1, dirPins[1], stepPins[1]); // right

Point start = Point(0, 0);
float *startStrings = start.getStrings();

Servo pen = Servo(penPin);

Plotter plotter = {
  .pos = start, 
  .strings = {
    startStrings[0], startStrings[1]
  },
  .stepper1 = stepper1,
  .stepper2 = stepper2,
  .pen =  pen,
};

void draw(std::string text) {
  // Wake motors up
  setMotorSleep(false);
  // Make sure the pen is up
  // Signal to start moving with joystick
  plotter.pen.penDown();
  plotter.pen.penUp();
  // Serial.printf("Move to start ...");
  plotter.joystick(false);
  // Serial.printf("Selected position: %f, %f\n", plotter.pos.x, plotter.pos.y);
  // Serial.println("Drawing svg ...");

  SVG svg = SVG(text);
  plotter.executeSVG(svg);

  // Serial.println("Drawing Finished!");
  // Return to start and set motors to sleep
  plotter.pen.penUp();
  plotter.moveTo(start);
  setMotorSleep(true);
}

void setup() {
  Serial.begin(9600);
  setMotorState(true);
  delay(2000);
}

void loop() {
  // SD card inserted
  if (SD.begin()) {
    // File needs to be named "draw.svg"
    File file = SD.open("/draw.svg", FILE_READ);
    if (!file) {
      Serial.println("Failed to open 'draw.svg'!"); 
    } else {
      std::string svgString;
      while (file.available()) {
        svgString.push_back(file.read());
      }
      draw(svgString);
    }
  }
  // Wait 1 sec and try again
  delay(1000);
}