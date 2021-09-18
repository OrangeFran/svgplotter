// #include <pthread.h>
#include <thread>
#include <Arduino.h>

#include "joystick.h"
#include "stepper.h"

// The whiteboard (in mm)
float boardWidth = 1930.0;
float boardHeight = 1150.0;
// The current position (s1, s2)
// s1 = left string, s2 = right string
float initialPosition = sqrt(pow(boardWidth/2, 2) + pow(boardHeight, 2));
float position[2] = {initialPosition, initialPosition};

float baseVelocity = 500.0;

StepperMotor stepper1 = StepperMotor(0, dirPins[0], stepPins[0]);
StepperMotor stepper2 = StepperMotor(1, dirPins[1], stepPins[1]);


typedef struct Point {
  float x;
  float y;
} Point;

// Move to a coordinate
int goTo(Point p) {
  Serial.printf("Going to (%d, %d) ...", p.x, p.y);

  // Calculate the new length of the string with the
  // Pythagoras formulae
  float newS1 = sqrt(pow(boardWidth/2 + p.x, 2) + pow(boardHeight - p.y, 2));
  float newS2 = sqrt(pow(boardWidth/2 - p.x, 2) + pow(boardHeight - p.y, 2));
  // Calculate the necessary movement
  float distanceS1 = newS1 - position[0];
  float distanceS2 = newS2 - position[1];
  // Update the position
  position[0] = newS1;
  position[1] = newS2;

  Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
  Serial.printf("New position:  %f, %f\n", position[0], position[1]);

  // Calculate the needed velocities
  float velocityS1, velocityS2;
  // The velocity for the shorter distance will
  // be a fraction of the base velocity of the longer distance
  if (distanceS1 < distanceS2) {
    velocityS1 = baseVelocity;
    velocityS2 = distanceS1 / distanceS2 * baseVelocity;
  } else {
    velocityS2 = baseVelocity;
    velocityS1 = distanceS2 / distanceS1 * baseVelocity;
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
  float steps = abs(distanceS1)/perstep;

  // // Wrap the member functions to fill the requirement for a
  // // static non-member function
  // Serial.println("Starting threads ...");
  // std::thread threadS1([](int d) { stepper1.travel(d); }, distanceS1);
  // std::thread threadS2([](int d) { stepper2.travel(d); }, distanceS2);
  // Serial.println("Waiting for threads ...");
  // threadS1.join();
  // threadS2.join();

  // // Use timer interrupts to stop the pwm signal
  // hw_timer_t *timerS1 = timerBegin(0, 80, true);
  // hw_timer_t *timerS2 = timerBegin(0, 80, true);
  // timerAttachInterrupt(timerS1, &[](){ stepper1.stop(); }, true);
  // timerAttachInterrupt(timerS2, &[](){ stepper2.stop(); }, true);
  // timerAlarmWrite(timerS1, velocityS1 * steps, false);
  // timerAlarmWrite(timerS2, velocityS2 * steps, false);

  stepper1.start();
  // timerAlarmEnable(timerS1);
  stepper2.start();
  // timerAlarmEnable(timerS2);

  delay((int)(stepper1.frequency/steps));

  stepper1.stop();
  stepper2.stop();

  return 0;
}

// Implementation of De Casteljau's algorithm
// parametric function with `t` -> move t from 0 to 1
// `B(t) = (1 - t)^2 P_0 + 2t (1 - t) P_1 + t^2 P_2`
int bezierCurve(Point p0, Point p1, Point p2) {
  int accuracy = 50;
  // Move t from 0 to 1
  // accuracy defines the amount of steps between
  int x, y;
  for (int t = 0; t <= 1; t += 1/accuracy) {
    x = pow((1 - t), 2) * p0.x + 2 * t * (1 - t) * p1.x + pow(t, 2) * p2.x;
    y = pow((1 - t), 2) * p0.y + 2 * t * (1 - t) * p1.y + pow(t, 2) * p2.y;
    if (goTo(x, y) != 0) {
      Serial.println("Failed to move!");
    }
  }
  return 0;
}

void setup() {
  Serial.begin(9600);
  setMotorState(true);
  // A simple square
  delay(5000);
  goTo(Point{0, 100});
  delay(5000);
  goTo(Point{100, 100});
  delay(5000);
  goTo(Point{100, 0});
  delay(5000);
  goTo(Point{0, 0});
}

void loop() {
  // Deattach pins
  ledcDetachPin(stepper1.stepPin);
  ledcDetachPin(stepper2.stepPin);
  // Fall back to joystick control
  joystick();
}