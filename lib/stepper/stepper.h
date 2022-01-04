// Header guard
// https://gcc.gnu.org/onlinedocs/cpp/Once-Only-Headers.html
#ifndef STEPPER_H
#define STEPPER_H

void setMotorState(bool);
void setMotorSleep(bool);

extern const float perstep;

extern const int penPin;
// Pins to set direction and move ({ leftPin, rightPin })
extern const int dirPins[2];
extern const int stepPins[2];

// A smaller class to StepperMotor
// This is needed to pass values and functions
// to the accel_timer callback
typedef struct Motor {
  int index; // Left motor -> 0, right motor -> 1
  float velocity; // Steps per second (= Hz)
  float target_velocity;
  float accel; 
  int timeLastCall;
  esp_timer_handle_t accel_timer;
  esp_timer_handle_t stop_timer;
  int stepsToDo;
  // Constructor
  Motor(int);
} Motor;

class StepperMotor {
  private:
    int dirPin;
    int stepPin;
    // Specify if step pin is attached to PWM channel or not
    // Joystick only works if attached is false
    bool attached;
    Motor *motor;

  public:
    int index; // Left motor -> 0, right motor -> 1

    StepperMotor(int, int, int);
    void step(void);
    void attachPin(void);
    void detachPin(void);
    void applyDirection(bool);
    // void applyVelocity(float);
    // PWM signal controls
    void doSteps(float, float, int);
};

#endif