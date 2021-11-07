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
  int delayS1, delayS2;

  // Special cases
  if (stepsS1 == 0 && stepsS2 == 0) {
    // Return without doing anything
    return 0;
  } else if (stepsS1 == 0) {
    delayS2 = baseVelocity; 

    // // Debug output?
    // Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    // Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    // Serial.printf("New position:  %f, %f\n", p.x, p.y);
    // Serial.printf("Velocity: %d\n", velocityS2);
    // Serial.printf("Delay for S2: %d\n", delayTimeS2);

    this->stepper2.setVelocity(delayS2, distanceS2 < 0);
    this->stepper2.start(stepsS2);

  } else if (stepsS2 == 0) {
    delayS1 = baseVelocity; 

    // // Debug output?
    // Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    // Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    // Serial.printf("New position:  %f, %f\n", p.x, p.y);
    // Serial.printf("Velocity: %d\n", velocityS1);
    // Serial.printf("Delay for S1: %d\n", delayTimeS1);

    this->stepper1.setVelocity(delayS1, distanceS1 < 0);
    this->stepper1.start(stepsS1);

  // Normal cases
  } else {
    if (stepsS1 > stepsS2) {
      delayS1 = baseVelocity;
      delayS2 = round((float)stepsS1/(float)stepsS2 * baseVelocity);
    } else if (stepsS1 < stepsS2) {
      delayS2 = baseVelocity;
      delayS1 = round((float)stepsS2/(float)stepsS1 * baseVelocity);
    } else {
      delayS1 = baseVelocity;
      delayS2 = baseVelocity;
    }

    // Debug output?
    Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    Serial.printf("New position:  %f, %f\n", p.x, p.y);
    Serial.printf("Delays: %d, %d\n", delayS1, delayS2);
    // Serial.printf("Delay for S1: %d\n", delayTimeS1);
    // Serial.printf("Delay for S2: %d\n", delayTimeS2);

    // NOTE: Other approaches tried:
    //         - seperate threads for each motor
    //           -> failed, because `vTaskDelay` is only able to
    //              delay with a precision of 1ms 

    this->stepper1.setVelocity(delayS1, distanceS1 < 0);
    this->stepper2.setVelocity(delayS2, distanceS2 < 0);

    Serial.println("Starting on core 0 ...");
    this->stepper1.start(stepsS1);
    Serial.println("Starting on core 1 ...");
    this->stepper2.start(stepsS2);
    Serial.println("Continuing ...");

    // // Wait for the timers to trigger a stop
    // int next_alarm;
    // while (true) {
    //   next_alarm = esp_timer_get_next_alarm() - esp_timer_get_time();
    //   if (next_alarm <= 0) {
    //     break;
    //   };
    //   delayMicroseconds(next_alarm);
    // }
  }

  // Update the position
  this->pos = p;
  this->strings[0] = newPos[0];
  this->strings[1] = newPos[1];

  return 0;
}

int Plotter::splitMove(Point p) {
  // Split a distance 10 times 
  float x, y;
  float increase[2] = { (p.x - this->pos.x) / 10.0, (p.y - this->pos.y) / 10.0 };
  // Serial.printf("Increases: %f, %f\n", increase[0], increase[1]);
  for (int i = 1; i <= 10; i++) {
    x = this->pos.x + increase[0];
    y = this->pos.y + increase[1];
    // Serial.printf("Moving to %f, %f ...\n", x, y);
    this->moveTo(Point(x, y));
    delay(100);
  }

  return 0;
}