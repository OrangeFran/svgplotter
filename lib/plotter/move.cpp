#include <Arduino.h>
#include "plotter.h"

// Move to a coordinate
int Plotter::moveTo(Point p) {
  // Serial.printf(
  //   "\bMoving from (%f, %f) to (%f, %f) ...",
  //   this->pos.x, this->pos.y, p.x, p.y
  // );

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

    // Debug output?
    Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
    Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
    Serial.printf("New position:  %f, %f\n", p.x, p.y);
    Serial.printf("Velocities: %d, %d\n", velocityS1, velocityS2);
    Serial.printf("Delay for S1: %d\n", delayTimeS1);
    Serial.printf("Delay for S2: %d\n", delayTimeS2);

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
  }

  // Update the position
  this->pos = p;
  this->strings[0] = newPos[0];
  this->strings[1] = newPos[1];

  return 0;
}