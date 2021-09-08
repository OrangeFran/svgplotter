#include <pthread.h>
#include <Arduino.h>

#include "joystick.h"
#include "stepper.h"

// The whiteboard (in mm)
int boardWidth = 1930;
int boardHeight = 1170;
// The current position (s1, s2)
// s1 = left string, s2 = right string
int position[2] = {1516, 1516};

float baseVelocity = 1.0;
// TODO: measure/calculate exact mm per step
float perstep = 0.019625; // 0.07925;

// int old_go_to(int x, int y) {
//   // Calculate the new length of the string
//   int new_s1 = sqrt(pow(width/2 + x, 2) + pow(height - y, 2));
//   int new_s2 = sqrt(pow(width/2 - x, 2) + pow(height - y, 2));
//   // Calculate the necessary movement
//   int distance_s1 = new_s1 - position[0];
//   int distance_s2 = new_s2 - position[1];
//   // Update the position
//   position[0] = new_s1;
//   position[1] = new_s2;
// 
//   Serial.printf("-> %d, %d\n", distance_s1, distance_s2);
//   Serial.printf("position: %d, %d\n", position[0], position[1]);
// 
//   struct travel_args to_travel_s1 = {distance_s1, 0};
//   struct travel_args to_travel_s2 = {distance_s2, 1};
//   travel(&to_travel_s1);
//   travel(&to_travel_s2);
//   return 0;
// }

int goTo(int x, int y) {
  Serial.printf("Going to (%d, %d) ...", x, y);

  // Calculate the new length of the string with the
  // Pythagoras formulae
  int newS1 = sqrt(pow(boardWidth/2 + x, 2) + pow(boardHeight - y, 2));
  int newS2 = sqrt(pow(boardWidth/2 - x, 2) + pow(boardHeight - y, 2));
  // Calculate the necessary movement
  int distanceS1 = newS1 - position[0];
  int distanceS2 = newS2 - position[1];
  // Update the position
  position[0] = newS1;
  position[1] = newS2;

  Serial.printf("Distances: %d, %d\n", distanceS1, distanceS2);
  Serial.printf("New position:  %d, %d\n", position[0], position[1]);

  // Calculate the needed velocities
  float velocityS1, velocityS2;
  if (distanceS1 < distanceS2) {
    velocityS1 = baseVelocity;
    velocityS2 = (float)abs(distanceS1) / (float)abs(distanceS2) * velocityS1;
  } else {
    velocityS2 = baseVelocity;
    velocityS1 = (float)abs(distanceS2) / (float)abs(distanceS1) * velocityS2;
  }

  Serial.printf("Velocities: %f, %f\n", velocityS1, velocityS2);

  // Spawn two threads, one for each motor
  pthread_t threads[2];
  int creationS1 = pthread_create(&threads[0], NULL, travel, &distanceS1);
  int creationS2 = pthread_create(&threads[1], NULL, travel, &distanceS2);
  if (creationS1 != 0 || creationS2 != 0) {
    Serial.println("Failed to create threads!");
    return 1;
  }

  // Wait for the threads to complete
  // and check the status code
  void *statusS1, *statusS2;
  pthread_join(threads[0], &statusS1);
  pthread_join(threads[1], &statusS2);
  if (statusS1 != 0 || statusS2 != 0) {
    Serial.println("Failed to move!");
    return 1;
  }

  return 0;
}

void setup() {
  Serial.begin(9600);
  // Set all pins to output
  motorState(true);
  for (int p = 0; p < 2; p++)
    pinMode(dirPins[p], OUTPUT);
  for (int p = 0; p < 2; p++)
    pinMode(stepPins[p], OUTPUT);
  // Move to a coordinate
  if (goTo(200, 100) != 0) {
    Serial.println("Could not move to coordinate!");
  }
}

void loop() {
  // Fall back to joystick control
  joystick();
}
