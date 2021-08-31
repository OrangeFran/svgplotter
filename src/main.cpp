#include <Arduino.h>
#include "joystick.h"
#include "move.h"

// The whiteboard (in mm)
int width = 1930;
int height = 1170;
// The current position (s1, s2)
// s1 = left string, s2 = right string
int position[] = {1535, 1535};

int velocity = 500;
// TODO: measure/calculate exact mm per step
float perstep = 0.019625; // 0.07925;

// Make the string for a certain motor longer/shorter
int directions[] = {1, 0};
void travel(int distance, int stepper) {
  // Set the direction
  if (distance < 0) {
    digitalWrite(dirPins[stepper], directions[stepper]);
  } else {
    digitalWrite(dirPins[stepper], stepper);
  }
  // Calculate and do steps
  int steps = abs(distance)/perstep;
  for (int i = 0; i < steps; i++) {
    step(stepper);
    delayMicroseconds(velocity);
  }
}

void goTo(int x, int y) {
  int new_s1 = sqrt(pow(width/2 + x, 2) + pow(height - y, 2));
  int new_s2 = sqrt(pow(width/2 - x, 2) + pow(height - y, 2));
  int distance_s1 = new_s1 - position[0];
  int distance_s2 = new_s2 - position[1];

  Serial.printf("-> %d, %d", distance_s1, distance_s2);

  // // TODO: move the direction part into the travel function
  // // If negative, go clockwise
  // if (distance_s1 < 0) {
  //   digitalWrite(dirPins[0], 1);
  // // Else turn anticlockwise
  // } else {
  //   digitalWrite(dirPins[0], 0);
  // }
  travel(distance_s1, 0);

  // // If string to is negative, go anticlockwise
  // if (distance_s2 < 0) {
  //   digitalWrite(dirPins[1], 0);
  // // Else turn clockwise
  // } else {
  //   digitalWrite(dirPins[1], 1);
  // }
  travel(distance_s2, 1);
}

void setup() {
  Serial.begin(9600);
  // Set all pins to output
  for (int p = 0; p < 2; p++)
    pinMode(dirPins[p], OUTPUT);
  for (int p = 0; p < 2; p++)
    pinMode(stepPins[p], OUTPUT);
  // pinMode(joyPins[2], INPUT_PULLUP);
  motorState(true);
  goTo(100, 100);
}

void loop() {
  joystick();
}
