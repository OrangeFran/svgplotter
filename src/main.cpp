#include <Arduino.h>
#include <SD.h>

#include "stepper.h"
#include "plotter.h"
#include "parser.h"
#include "file.h"

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

void draw(CustomStream *stream, bool scale) {
  // Wake motors up
  setMotorSleep(false);
  // Make sure the pen is up
  // Signal to start moving with joystick
  plotter.makePoint();

  SVG svg = SVG(stream);

  plotter.joystick(false);
  // if (scale) {
    // plotter.makePoint();
    // // Save the position to drive back
    // Point origin = plotter.pos;
    // // Let user draw line
    // plotter.joystick(false);
    // plotter.makePoint();
    // // Save the position
    // Point end = plotter.pos;
    // // Serial.printf("end: (%f, %f)", end.x, end.y);
    // // Move back
    // plotter.moveTo(origin);

    // // Calculate degree and length
    // float dx = end.x - origin.x;
    // float dy = end.y - origin.y;
    // float degree = atan(dy/dx);
    // // Add 2π if degree negative
    // if (degree < 0) {
      // degree += 2 * PI;
    // }
    // float length = sqrt(pow(dx, 2) + pow(dy, 2));
    // svg.scale(length);
  // }

  // svg.rotate(degree);
  plotter.executeSVG(svg);

  // Return to start and set motors to sleep
  plotter.pen.penUp();
  plotter.moveTo(start);
  setMotorSleep(true);
}

// Extract file extension
// Ex. 'draw.svg' -> '.svg'
std::string findExtension(char *name) {
  std::string extension;
  Serial.printf("Finding extension of '%s'!", name);
  for (int i = 0; i < strlen(name); i++) {
    if (name[i] == '.') {
      Serial.printf("Found point\n");
      extension = name + i;
      break;
    }
  }
  return extension;
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
      // List files and find svgs
      // Make sure the files have a ".svg" extension
      File root = SD.open("/", FILE_READ);
      while (true) {
        File f = root.openNextFile();
        // Checked all files
        if (!f) { break; }
        // Find extension
        const char *name = f.name();
        std::string extension = findExtension(name);

        // Draw file if extension matches
        Serial.printf("%d\n", strcmp(extension, ".svg"));
        if (strcmp(extension, ".svg") == 0) {
          Serial.printf("Drawing '%s' ...\n", name);
          CustomStream *sstream = new FileStream(f);
          draw(sstream, false);
        } else {
          Serial.printf("Skipping '%s' ...\n", name);
        }

        f.close();
        delay(1000);
      }
    }
    // Wait 1 sec and try again
    delay(1000);
  }
}

void loop() {}