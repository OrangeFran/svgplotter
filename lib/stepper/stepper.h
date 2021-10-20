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
    int index; 
    int dirPin;
    int stepPin;

    int accel;
    int stepsToDo;
    // In steps per second (= Hz)
    int start_velocity;
    int current_velocity;
    int target_velocity;

    void step(void);
    void setVelocity(int);

  public:
    esp_timer_handle_t stop_timer;
    esp_timer_handle_t accel_timer;

    // Constructor
    StepperMotor(int, int, int);
    // PWM signal controls
    int setAcceleration(int, int, bool);
    int start(int);
};

// Old function still needed for lib/joystick
void step(int);

#endif