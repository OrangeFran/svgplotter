#include <Arduino.h>
#include "plotter.h"

// Move to a coordinate
int Plotter::moveTo(Point p) {
  // Calculate the necessary movement
  float *newPos = p.calculatePosition(); 
  Serial.printf("New pos: %f, %f", newPos[0], newPos[1]);
  float distanceS1 = newPos[0] - this->stringLength[0];
  float distanceS2 = newPos[1] - this->stringLength[1];
  // Update the position
  this->pos = p;
  this->stringLength[0] = newPos[0];
  this->stringLength[1] = newPos[1];

  // TODO: round() or int()
  int stepsS1 = round(abs(distanceS1)/perstep);
  int stepsS2 = round(abs(distanceS2)/perstep);

  int velocityS1, velocityS2;
  // The velocity for the shorter distance will
  // be a fraction of the base velocity of the longer distance
  if (stepsS1 > stepsS2) {
    velocityS1 = baseVelocity;
    velocityS2 = round((float)stepsS2/(float)stepsS1 * baseVelocity);
  } else {
    velocityS2 = baseVelocity;
    velocityS1 = round((float)stepsS1/(float)stepsS2 * baseVelocity);
  }

  int delayTimeS1 = round((float)stepsS1/(float)velocityS1 * 1000000);
  int delayTimeS2 = round((float)stepsS2/(float)velocityS2 * 1000000);

  Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
  Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
  Serial.printf("New position:  %f, %f\n", this->pos.x, this->pos.y);
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
  delayMicroseconds(esp_timer_get_next_alarm() - esp_timer_get_time());
  // NOTE: Needed?
  int next_alarm = esp_timer_get_next_alarm() - esp_timer_get_time();
  if (next_alarm > 0) {
    delayMicroseconds(next_alarm);
  };

  return 0;
}