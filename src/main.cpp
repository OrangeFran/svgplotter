// #include <pthread.h>
#include <thread>
#include <Arduino.h>

#include "joystick.h"
#include "stepper.h"

// The whiteboard (in mm)
int boardWidth = 1930;
int boardHeight = 1150;
// The current position (s1, s2)
// s1 = left string, s2 = right string
int initialPosition = sqrt(pow(boardWidth/2, 2) + pow(boardHeight, 2));
int position[2] = {initialPosition, initialPosition};

float baseVelocity = 500;

StepperMotor stepper1 = StepperMotor(0, dirPins[0], stepPins[0]);
StepperMotor stepper2 = StepperMotor(1, dirPins[1], stepPins[1]);

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

  stepper1.setVelocity(velocityS1);
  stepper2.setVelocity(velocityS2);

  // // Spawn two threads, one for each motor
  // pthread_t threads[2];
  // int creationS1 = pthread_create(&threads[0], NULL, [](int d) { stepper1.travel(d); }, &distanceS1);
  // int creationS2 = pthread_create(&threads[1], NULL, [](int d) { stepper2.travel(d); }, &distanceS2);
  // if (creationS1 != 0 || creationS2 != 0) {
  //   Serial.println("Failed to create threads!");
  //   return 1;
  // }

  // // Wait for the threads to complete
  // // and check the status code
  // void *statusS1, *statusS2;
  // pthread_join(threads[0], &statusS1);
  // pthread_join(threads[1], &statusS2);
  // if (statusS1 != 0 || statusS2 != 0) {
  //   Serial.println("Failed to move!");
  //   return 1;
  // }

  // Calculate and do steps
  // The direction is already set, so the prefix can be
  // removed with abs()
  int steps = abs(distanceS1)/perstep;

  // Wrap the member functions to fill the requirement for a
  // static non-member function
  Serial.println("Starting threads ...");
  std::thread threadS1([](int d) { stepper1.travel(d); }, distanceS1);
  std::thread threadS2([](int d) { stepper2.travel(d); }, distanceS2);

  Serial.println("Waiting for threads ...");
  threadS1.join();
  threadS2.join();

  delayMicroseconds(velocityS1 * steps);
  stepper1.stop();
  stepper2.stop();

  return 0;
}

// typedef struct Point {
//   float x;
//   float y;
// } Point;

// // Implementation of De Casteljau's algorithm
// // parametric function with `t` -> move t from 0 to 1
// //
// // `B(t) = (1 - t)^2 P_0 + 2t (1 - t) P_1 + t^2 P_2`

// int bezierCurve(Point p0, Point p1, Point p2) {
//   int accuracy = 50;
//   // Move t from 0 to 1
//   // accuracy defines the amount of steps between
//   int x, y;
//   for (int t = 0; t =< 1; t += 1/accuracy) {
//     x = pow((1 - t), 2) * p0.x + 2 * t * (1 - t) * p1.x + pow(t, 2) * p2.x;
//     y = pow((1 - t), 2) * p0.y + 2 * t * (1 - t) * p1.y + pow(t, 2) * p2.y;
//     if (goTo(x, y) != 0) {
//       Serial.println("Failed to move!");
//     }
//   }
//   return 0;
// }

void setup() {
  Serial.begin(9600);
  setMotorState(true);
  // Move to a coordinate
  delay(5000);
  goTo(0, 100);
  delay(5000);
  goTo(100, 100);
  delay(5000);
  goTo(100, 0);
  delay(5000);
  goTo(0, 0);
}

void loop() {
  // Deattach pins
  ledcDetachPin(stepper1.stepPin);
  ledcDetachPin(stepper2.stepPin);
  // Fall back to joystick control
  joystick();
}