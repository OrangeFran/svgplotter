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
  public:
    // Left motor -> 0, right motor -> 1
    int index; 
    // In steps per second (= Hz)
    float velocity; 
    esp_timer_handle_t timer;
    int dirPin;
    int stepPin;

    // Constructor
    StepperMotor(int, int, int);
    void step(void);
    void setVelocity(int, bool);
    // PWM signal controls
    int start(int);
    int stop(void);
};

// Old function still needed for lib/joystick
void step(int);

#endif