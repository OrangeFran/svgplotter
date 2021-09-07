#include <pthread.h>
#include <Arduino.h>

#include "joystick.h"
#include "move.h"

// The whiteboard (in mm)
int board_width = 1930;
int board_height = 1170;
// The current position (s1, s2)
// s1 = left string, s2 = right string
int position[2] = {1516, 1516};

float base_velocity = 1.0;
// TODO: measure/calculate exact mm per step
float perstep = 0.019625; // 0.07925;

struct travel_args {
  float velocity;
  int distance;
  int stepper;
};
// Make the string for a certain motor longer/shorter
void *travel(void *args) {
  int stepper = ((struct travel_args *)args)->stepper;
  int distance = ((struct travel_args *)args)->distance;
  float velocity = ((struct travel_args *)args)->velocity;

  // Set the direction
  digitalWrite(dirPins[stepper], distance > 0 ? stepper : (int)!(bool)stepper);
  // Calculate and do steps
  int steps = abs(distance)/perstep;
  TickType_t x_last_wake_time = xTaskGetTickCount();
  for (int i = 0; i < steps; i++) {
    step(stepper);
    // delayMicroseconds will not work,
    // because it does not provide time for the watchdog
    // vTaskDelay is absolute, vTaskDelayUntil is relative
    vTaskDelayUntil(&x_last_wake_time, velocity * portTICK_PERIOD_MS);
    // vTaskDelay(velocity * portTICK_PERIOD_MS);
  }

  return 0;
}

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

int go_to(int x, int y) {
  Serial.printf("Going to (%d, %d) ...", x, y);

  // Calculate the new length of the string
  int new_s1 = sqrt(pow(board_width/2 + x, 2) + pow(board_height - y, 2));
  int new_s2 = sqrt(pow(board_width/2 - x, 2) + pow(board_height - y, 2));
  // Calculate the necessary movement
  int distance_s1 = new_s1 - position[0];
  int distance_s2 = new_s2 - position[1];
  // Update the position
  position[0] = new_s1;
  position[1] = new_s2;

  Serial.printf("Distances: %d, %d\n", distance_s1, distance_s2);
  Serial.printf("New position:  %d, %d\n", position[0], position[1]);

  // Calculate the needed velocities
  float velocity_s1, velocity_s2;
  if (distance_s1 < distance_s2) {
    velocity_s1 = base_velocity;
    velocity_s2 = (float)abs(distance_s1) / (float)abs(distance_s2) * velocity_s1;
  } else {
    velocity_s2 = base_velocity;
    velocity_s1 = (float)abs(distance_s2) / (float)abs(distance_s1) * velocity_s2;
  }

  Serial.printf("Velocities: %f, %f\n", velocity_s1, velocity_s2);

  // Spawn two threads, one for each motor
  pthread_t threads[2];
  struct travel_args to_travel_s1 = {velocity_s1, distance_s1, 0};
  struct travel_args to_travel_s2 = {velocity_s2, distance_s2, 1};
  int return_val_s1 = pthread_create(&threads[0], NULL, travel, &to_travel_s1);
  int return_val_s2 = pthread_create(&threads[1], NULL, travel, &to_travel_s2);
  if (return_val_s1 != 0 || return_val_s2 != 0) {
    Serial.println("Failed to create threads!");
    return 1;
  }

  // Wait for the threads to complete
  // and check the status code
  void *status_s1, *status_s2;
  pthread_join(threads[0], &status_s1);
  pthread_join(threads[1], &status_s2);
  if (status_s1 != 0 || status_s2 != 0) {
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
  if (go_to(200, 100) != 0) {
    Serial.println("Could not move to coordinate!");
  }
}

void loop() {
  joystick();
}
