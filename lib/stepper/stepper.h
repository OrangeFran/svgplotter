// Header guard
// https://gcc.gnu.org/onlinedocs/cpp/Once-Only-Headers.html
#ifndef STEPPER_H
#define STEPPER_H

void setMotorState(bool);

extern const float perstep;
// Global turned on state of motors
extern bool motorState;

extern const int penPin;
// Pins to set direction and move
// { leftPin, rightPin }
extern const int dirPins[2];
extern const int stepPins[2];

// One stepper motor
class StepperMotor {
  private:
    int dirPin;
    int stepPin;
    xTaskHandle *task;

  public:
    int index; // Left motor -> 0, right motor -> 1
    int delay; // Delay in microseconds
    int _steps;

    StepperMotor(int, int, int);

    void step(void);
    void setDirection(bool);
    void setVelocity(int, bool);
    // PWM signal controls
    void start(int);
    void stop(void);
};

#endif