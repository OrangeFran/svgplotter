#include <Arduino.h>
#include "plotter.h"

// Move to a coordinate
int Plotter::moveTo(Point p) {
  // Calculate the necessary movement
  float *newPos = p.getStrings(); 
  float distanceS1 = newPos[0] - this->strings[0];
  float distanceS2 = newPos[1] - this->strings[1];

  int stepsS1 = round(abs(distanceS1)/perstep);
  int stepsS2 = round(abs(distanceS2)/perstep);

  // The velocity for the shorter distance will
  // be a fraction of the base velocity of the longer distance
  int velocityS1, velocityS2;

  // Special cases
  if (stepsS1 == 0 && stepsS2 == 0) {
    // Return without doing anything
    return 0;
  } else if (stepsS1 == 0) {
    velocityS2 = baseVelocity; 
    int delayTimeS2 = round((float)stepsS2/(float)velocityS2 * 1000000);

    // // Debug output?
    // Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    // Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    // Serial.printf("New position:  %f, %f\n", p.x, p.y);
    // Serial.printf("Velocity: %d\n", velocityS2);
    // Serial.printf("Delay for S2: %d\n", delayTimeS2);

    this->stepper2.setVelocity(velocityS2, distanceS2 < 0);
    this->stepper2.start(delayTimeS2);
    // Wait for the timers to trigger a stop
    delayMicroseconds(esp_timer_get_next_alarm() - esp_timer_get_time());

  } else if (stepsS2 == 0) {
    velocityS1 = baseVelocity; 
    int delayTimeS1 = round((float)stepsS1/(float)velocityS1 * 1000000);

    // // Debug output?
    // Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    // Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    // Serial.printf("New position:  %f, %f\n", p.x, p.y);
    // Serial.printf("Velocity: %d\n", velocityS1);
    // Serial.printf("Delay for S1: %d\n", delayTimeS1);

    this->stepper1.setVelocity(velocityS1, distanceS1 < 0);
    this->stepper1.start(delayTimeS1);
    // Wait for the timers to trigger a stop
    delayMicroseconds(esp_timer_get_next_alarm() - esp_timer_get_time());

  // Normal cases
  } else {
    if (stepsS1 > stepsS2) {
      velocityS1 = baseVelocity;
      velocityS2 = round((float)stepsS2/(float)stepsS1 * baseVelocity);
    } else if (stepsS1 < stepsS2) {
      velocityS2 = baseVelocity;
      velocityS1 = round((float)stepsS1/(float)stepsS2 * baseVelocity);
    } else {
      velocityS1 = baseVelocity;
      velocityS2 = baseVelocity;
    }

    int delayTimeS1 = round((float)stepsS1/(float)velocityS1 * 1000000);
    int delayTimeS2 = round((float)stepsS2/(float)velocityS2 * 1000000);

    // // Debug output?
    // Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    // Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    // Serial.printf("New position:  %f, %f\n", p.x, p.y);
    // Serial.printf("Velocities: %d, %d\n", velocityS1, velocityS2);
    // Serial.printf("Delay for S1: %d\n", delayTimeS1);
    // Serial.printf("Delay for S2: %d\n", delayTimeS2);

    // NOTE: Other approaches tried:
    //         - seperate threads for each motor
    //           -> failed, because `vTaskDelay` is only able to
    //              delay with a precision of 1ms 

    this->stepper1.setVelocity(velocityS1, distanceS1 < 0);
    this->stepper2.setVelocity(velocityS2, distanceS2 < 0);

    this->stepper1.start(delayTimeS1);
    this->stepper2.start(delayTimeS2);

    // Wait for the timers to trigger a stop
    int next_alarm;
    while (true) {
      next_alarm = esp_timer_get_next_alarm() - esp_timer_get_time();
      if (next_alarm <= 0) {
        break;
      };
      delayMicroseconds(next_alarm);
    }

    // Cautionary waiting
    delay(100);
  }

  // Update the position
  this->pos = p;
  this->strings[0] = newPos[0];
  this->strings[1] = newPos[1];

  return 0;
}

int Plotter::splitMove(Point p) {
  float x, y;
  Point start = this->pos;
  float diffs[2] = { p.x - start.x, p.y - start.y };
  float length = sqrt(pow(diffs[0], 2) + pow(diffs[1], 2));

  // Split a distance into 20mm pieces
  float accuracy = length/20;
  float increase = 1/accuracy;

  // Serial.printf("Diffs: %f, %f\n", diffs[0], diffs[1]);
  // Serial.printf("Length: %f\n", length);
  // Serial.printf("Accuracy: %f\n", accuracy);
  // Serial.printf("Increase: %f\n", increase);

  for (float t = increase; t <= (float)1.0; t += increase) {
    // Serial.printf("T: %f\n", t);
    // Serial.printf("Pos: %f, %f\n", start.x, start.y);
    x = start.x + t * diffs[0];
    y = start.y + t * diffs[1];
    // Serial.printf("Moving to %f,%f\n", x, y);
    this->moveTo(Point(x, y));
    // delay(100);
  }

  // Move to end position
  // if all increases did not exactly add up to 1.0
  this->moveTo(p);

  return 0;
}