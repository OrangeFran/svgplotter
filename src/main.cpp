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

// Setup plotter
Plotter plotter = {
  .pos = start, 
  .strings = {
    startStrings[0], startStrings[1]
  },
  .stepper1 = stepper1,
  .stepper2 = stepper2,
  .pen =  pen,
};

void draw(CustomStream *stream, bool transoform) {
  // Wake motors up
  setMotorSleep(false);
  // Make sure the pen is up
  // Signal to start moving with joystick
  plotter.makePoint();

  SVG svg = SVG(stream);

  plotter.joystick(false);
  if (transform) {
    plotter.makePoint();
    // Save the position to drive back
    Point origin = plotter.pos;
    // Let user draw line
    plotter.joystick(false);
    plotter.makePoint();
    // Save the position
    Point end = plotter.pos;
    // Move back
    plotter.moveTo(origin);
    plotter.makePoint();

    float dx = end.x - origin.x;
    float dy = end.y - origin.y;

    // Calculate scaling 
    float length = sqrt(pow(dx, 2) + pow(dy, 2));
    svg.setScaleFactor(length/svg.viewBox[2]);

    // Calculate rotation degree
    float degree = atan(dy/dx);
    // Add π if dx negative (for 2. and 3. quadrant)
    if (dx < 0) {
      degree += PI;
    }
    svg.setRotation(degree);
  }

  plotter.executeSVG(svg);

  // Return to start and set motors to sleep
  plotter.pen.penUp();
  plotter.moveTo(start);
  setMotorSleep(true);
}

// Extract file extension
// Ex. 'draw.svg' -> '.svg'
const char *findExtension(const char *name) {
  const char *extension = (char *)malloc(sizeof(name));
  for (int i = 0; i < strlen(name); i++) {
    if (name[i] == '.') {
      extension = name + i;
      break;
    }
  }
  return extension;
}

void readFromSD(bool transform) {
  // Setup sd card
  // (https://www.instructables.com/Select-SD-Interface-for-ESP32/)
  // Pins: 13 (CS), 2 (MIS0), 14 (CLK), 15 (MOSI)
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
        // Stop if all files were checked
        if (!f) { 
          Serial.println("All available files have been drawn!");
          return;
        }

        const char *name = f.name();
        // Skip hidden files
        if (strncmp(name, "/.", 2) == 0) {
          Serial.printf("Skipping '%s' (hidden file) ...\n", name);
          continue;
        }
        // Find extension
        const char *extension = findExtension(name);

        // Draw file if extension matches
        if (strncmp(extension, ".svg", 4) == 0) {
          Serial.printf("Drawing '%s' ...\n", name);
          CustomStream *sstream = new FileStream(f);
          draw(sstream, transform);
          Serial.printf("'%s' drawn!\n", name);
        } else {
          Serial.printf("Skipping '%s' (no '.svg' extension) ...\n", name);
        }
        // Close the file and try the next
        f.close();
      }
    }
    // Wait 1 sec and try again
    delay(1000);
  }
}

void setup() {
  Serial.begin(9600);
  setMotorState(true);
  delay(2000);

  // Start SD card reader
  readFromSD(true);
}

void loop() {}