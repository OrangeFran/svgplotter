// #include <pthread.h>
#include <thread>
#include <Arduino.h>

#include "joystick.h"
#include "stepper.h"
#include "parser.h"

// The whiteboard (in mm)
float boardWidth = 1930.0;
float boardHeight = 1170.0;
// The current position (s1, s2)
// s1 = left string, s2 = right string
float initialPosition = sqrt(pow(boardWidth/2, 2) + pow(boardHeight, 2));
float position[2] = {initialPosition, initialPosition};

// In steps per second (sps)
// Delay of 500 µs in rps: 2000 sps
float baseVelocity = 2000.0;

StepperMotor stepper1 = StepperMotor(0, dirPins[0], stepPins[0]);
StepperMotor stepper2 = StepperMotor(1, dirPins[1], stepPins[1]);

typedef struct Point {
  float x;
  float y;
} Point;

// Move to a coordinate
int goTo(Point p) {
  // Calculate the new length of the string with the
  // Pythagoras formulae
  float newS1 = sqrt(pow(boardWidth/2 + p.x, 2) + pow(boardHeight - p.y, 2));
  float newS2 = sqrt(pow(boardWidth/2 - p.x, 2) + pow(boardHeight - p.y, 2));
  // Calculate the necessary movement
  float distanceS1 = newS1 - position[0];
  float distanceS2 = newS2 - position[1];

  // TODO: round() or int()
  int stepsS1 = round(abs(distanceS1)/perstep);
  int stepsS2 = round(abs(distanceS2)/perstep);

  // Update the position
  position[0] = newS1;
  position[1] = newS2;

  Serial.printf("Steps: %d, %d\n", stepsS1, stepsS2);
  Serial.printf("Distances: %f, %f\n", distanceS1, distanceS2);
  Serial.printf("New position:  %f, %f\n", position[0], position[1]);

  // Calculate the needed velocities
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

  Serial.printf("Velocities (sps): %d, %d\n", velocityS1, velocityS2);

  stepper1.setVelocity(velocityS1, distanceS1 < 0);
  stepper2.setVelocity(velocityS2, distanceS2 < 0);

  // NOTE: Other approaches tried:
  //         - seperate threads for each motor
  //           -> failed, because `vTaskDelay` is only able to
  //              delay with a precision of 1ms 


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

  // // Wrap the member functions to fill the requirement for a
  // // static non-member function
  // Serial.println("Starting threads ...");
  // std::thread threadS1([](int d) { stepper1.travel(d); }, distanceS1);
  // std::thread threadS2([](int d) { stepper2.travel(d); }, distanceS2);
  // Serial.println("Waiting for threads ...");
  // threadS1.join();
  // threadS2.join();

  int delayTimeS1 = round((float)stepsS1/(float)velocityS1 * 1000000);
  int delayTimeS2 = round((float)stepsS2/(float)velocityS2 * 1000000);

  Serial.printf("Delay time for S1: %d\n", delayTimeS1);
  Serial.printf("Delay time for S2: %d\n", delayTimeS2);

  stepper1.start(delayTimeS1);
  stepper2.start(delayTimeS2);

  // Wait for the timers to trigger a stop
  delayMicroseconds(esp_timer_get_next_alarm() - esp_timer_get_time());

  // // NOTE: For long distances, the delay times differ heavily.
  // //       Reason: The velocity needs to be specified in Hz. If the
  // //       can only be specified as an integer, one can loose/gain up
  // //       to 0.5 steps per second. This is the reason for the
  // //       two `delayMicroseconds`.
  // if (delayTimeS1 > delayTimeS2) {
  //   delayMicroseconds(delayTimeS2);
  //   stepper2.stop();
  //   delayMicroseconds(delayTimeS1 - delayTimeS2);
  //   stepper1.stop();
  // } else {
  //   delayMicroseconds(delayTimeS1);
  //   stepper1.stop();
  //   delayMicroseconds(delayTimeS2 - delayTimeS1);
  //   stepper2.stop();
  // }

  // If this does not work, split one big interval
  // into several smaller intervals -> rounding of velocity (max 0.5 steps)
  // does not influence the outcome because every interval is shorter than one second
  // To be even more exact, one could round the velocity in one interval up and in the following down

  return 0;
}

// Implementation of De Casteljau's algorithm
// parametric function with `t` -> move t from 0 to 1
// `B(t) = (1 - t)^2 P_0 + 2t (1 - t) P_1 + t^2 P_2`
int bezierCurve(Point p0, Point p1, Point p2) {
  // `accuracy` defines the amount of steps between
  float x, y;
  float increase = 1.0/10.0;
  // Move parameter t from 0.0 to 1.0
  for (float t = increase; t < 1.0 + increase; t += increase) {
    x = pow((1.0 - t), 2) * p0.x + 2.0 * t * (1.0 - t) * p1.x + pow(t, 2) * p2.x;
    y = pow((1.0 - t), 2) * p0.y + 2.0 * t * (1.0 - t) * p1.y + pow(t, 2) * p2.y;
    Serial.printf("Points: %f, %f\n", x, y);
    Serial.printf("T: %f\n", t);
    if (goTo(Point{x, y}) != 0) {
      Serial.println("Failed to move!");
    }
    delay(100);
  }
  return 0;
}

void setup() {
  Serial.begin(9600);
  setMotorState(true);

  // A simple square
  // delay(5000);
  // goTo(Point{0, 100});
  // delay(5000);
  // goTo(Point{100, 100});
  // delay(5000);
  // goTo(Point{100, 0});
  // delay(5000);
  // goTo(Point{0, 0});

  // delay(5000);
  // goTo(Point{100, 100});
  delay(1000);
  bezierCurve(Point{0, 0}, Point{100, 100}, Point{200, 0});
  // delay(1000);
  // bezierCurve(Point{100, 100}, Point{50, 50}, Point{0, 0});

  // Disconnect the pins from the PWM signal
  ledcDetachPin(stepper1.stepPin);
  ledcDetachPin(stepper2.stepPin);
}

void loop() {
  // Fall back to joystick control
  joystick();
}