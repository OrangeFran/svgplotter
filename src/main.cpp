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
  plotter.makePoint();

  // Serial.printf("Move to start ...");
  plotter.joystick(false);
  plotter.makePoint();
  // Save the position to drive back
  Point origin = plotter.pos;
  // Serial.printf("origin: (%f, %f)", origin.x, origin.y);
  // Let user draw line
  plotter.joystick(false);
  plotter.makePoint();
  // Save the position
  Point end = plotter.pos;
  // Serial.printf("end: (%f, %f)", end.x, end.y);
  // Move back
  plotter.moveTo(origin);

  // Calculate degree and length
  float dx = end.x - origin.x;
  float dy = end.y - origin.y;
  float degree = atan(dy/dx);
  // Add 2π if degree negative
  if (degree < 0) {
    degree += 2 * PI;
  }
  float length = sqrt(pow(dx, 2) + pow(dy, 2));
  // Serial.printf("dx: %f, dy: %f, degree: %f, length: %f\n", dx, dy, degree, length);
  // Serial.printf("Selected position: %f, %f\n", plotter.pos.x, plotter.pos.y);
  // Serial.println("Drawing svg ...");

  SVG svg = SVG(text);
  svg.scale(length);
  // svg.rotate(degree);
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

  // Setup sd card (https://www.instructables.com/Select-SD-Interface-for-ESP32/)
  // 13 (CS), 2 (MIS0), 14 (CLK), 15 (MOSI)
  SPIClass sdSPI = SPIClass(HSPI);
  sdSPI.begin(14, 2, 15, 13);
  while (true) {
    // SD card inserted
    if (SD.begin(13, sdSPI)) {
      // File needs to be named "draw.svg"
      File file = SD.open("/draw.svg", FILE_READ);
      if (!file) {
        Serial.println("Failed to open 'draw.svg'!"); 
      } else {
        Serial.println("Reading 'draw.svg' ..."); 
        std::string svgString;
        while (file.available()) {
          svgString.push_back(file.read());
        }
        Serial.println("Drawing 'draw.svg' ...");
        draw(svgString);
      }
    }
    // Wait 1 sec and try again
    delay(1000);
  }
}

void loop() {
}