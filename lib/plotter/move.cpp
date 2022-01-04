#include <Arduino.h>
#include "plotter.h"

void Plotter::makePoint() {
  this->pen.penDown();
  this->pen.penUp();
}

// Move to a coordinate
void Plotter::moveTo(Point p) {
  // Serial.printf("Moving to %f, %f", p.x, p.y);
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
    return;
  } else if (stepsS1 == 0) {
    velocityS2 = baseVelocity; 

    // // Debug output?
    // Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    // Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    // Serial.printf("New position:  %f, %f\n", p.x, p.y);
    // Serial.printf("Velocity: %d\n", velocityS2);
    // Serial.printf("Delay: %d\n", accelerationS2);

    this->stepper2.applyDirection(distanceS2 < 0);
    this->stepper2.doSteps(velocityS2, stepsS2);

  } else if (stepsS2 == 0) {
    velocityS1 = baseVelocity; 

    // // Debug output?
    // Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    // Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    // Serial.printf("New position:  %f, %f\n", p.x, p.y);
    // Serial.printf("Velocity: %d\n", velocityS1);
    // Serial.printf("Delay: %d\n", accelerationS1);

    this->stepper1.applyDirection(distanceS1 < 0);
    this->stepper1.doSteps(velocityS1, stepsS1);

  // Normal cases
  } else {
    if (stepsS1 > stepsS2) {
      velocityS1 = baseVelocity;
      velocityS2 = round((float)stepsS2/(float)stepsS1 * baseVelocity);
      // // Ledc can't produce frequency of 1 Hz without special configuration
      // if (round(velocityS1) < 2) {
      //   this->stepper2.detachPin();
      //   for (int i = 0; i < stepsS2; i++) {
      //     this->stepper2.step();
      //   }
      //   this->stepper2.attachPin();
      //   stepsS2 = 0;
      // } else {
      //   accelerationS2 = round((float)velocityS2/(float)velocityS1 * baseAcceleration);
      // }
    } else if (stepsS1 < stepsS2) {
      velocityS2 = baseVelocity;
      velocityS1 = round((float)stepsS1/(float)stepsS2 * baseVelocity);
      // // Ledc can't produce frequency of 1 Hz without special configuration
      // if (round(velocityS1) < 2) {
      //   this->stepper1.detachPin();
      //   for (int i = 0; i < stepsS1; i++) {
      //     this->stepper1.step();
      //   }
      //   this->stepper1.attachPin();
      // } else {
      //   accelerationS1 = round((float)velocityS1/(float)velocityS2 * baseAcceleration);
      // }
    } else {
      velocityS1 = baseVelocity;
      velocityS2 = baseVelocity;
    }

    // // Debug output?
    // Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    // Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    // Serial.printf("New position:  %f, %f\n", p.x, p.y);
    // Serial.printf("Velocities: %d, %d\n", velocityS1, velocityS2);
    // Serial.printf("Accelerations: %d, %d\n", accelerationS1, accelerationS2);

    // NOTE: Other approaches tried:
    //         - seperate threads for each motor
    //           -> failed, because `vTaskDelay` is only able to
    //              delay with a precision of 1ms 

    this->stepper1.applyDirection(distanceS1 < 0);
    this->stepper1.doSteps(velocityS1, stepsS1);

    this->stepper2.applyDirection(distanceS2 < 0);
    this->stepper2.doSteps(velocityS2, stepsS2);
  }

  // Wait for the timers to trigger a stop
  // `esp_timer_get_next_alarm()` is -1 if no timer is active
  int next_alarm;
  while (true) {
    next_alarm = esp_timer_get_next_alarm();
    if (next_alarm == -1) {
      // Serial.println("Breaking ...");
      break;
    };
    delay(100);
  }

  // Correct steps
  if (this->stepper1.motor->stepsToDo < 0) {
    this->stepper1.detachPin();
    // Reverse direction
    this->stepper1.applyDirection(distanceS1 > 0);
    this->stepper1.step();
    this->stepper1.attachPin();
    // Serial.println("Corrected s1");
  }
  if (this->stepper2.motor->stepsToDo < 0) {
    this->stepper2.detachPin();
    // Reverse direction
    this->stepper2.applyDirection(distanceS2 > 0);
    this->stepper2.step();
    this->stepper2.attachPin();
    // Serial.println("Corrected s2");
  }

  // Cautionary waiting
  delay(100);

  // Update the position
  this->pos = p;
  this->strings[0] = newPos[0];
  this->strings[1] = newPos[1];

  return;
}

void Plotter::splitMove(Point p) {
  float x, y;
  Point start = this->pos;

  // { dx, dy }
  float diffs[2] = { p.x - start.x, p.y - start.y };
  // The total length of the distance 
  float length = sqrt(pow(diffs[0], 2) + pow(diffs[1], 2));

  // Split a distance into 20mm pieces
  int accuracy = ceil(length/20.0);
  float increase = 1.0/(float)accuracy;

  float t = 0.0;
  for (int i = 0; i < accuracy; i++) {
    t += increase;
    x = start.x + t * diffs[0];
    y = start.y + t * diffs[1];
    this->moveTo(Point(x, y));
  }
}