// Header guard
// https://gcc.gnu.org/onlinedocs/cpp/Once-Only-Headers.html
#ifndef STEPPER_H
#define STEPPER_H

void setMotorState(bool);
void setMotorSleep(bool);

extern const float perstep;
// Global turned on state of motors
// extern bool motorState;

extern const int penPin;
// Pins to set direction and move ({ leftPin, rightPin })
extern const int dirPins[2];
extern const int stepPins[2];

typedef struct accelParameters {
  float accel;
  float target_velocity;
  float stepsToDo;
} AccelParameters;

class StepperMotor {
  public:
    int dirPin;
    int stepPin;
    esp_timer_handle_t accel_timer;
    esp_timer_handle_t stop_timer;
    // Specify if step pin is attached to PWM channel or not
    // Joystick only works if attached is false
    bool attached;

    int stepsToDo;
    float target_velocity;

  // public:
    int index; // Left motor -> 0, right motor -> 1
    float velocity; // Steps per second (= Hz)
    float accel; 

    StepperMotor(int, int, int);

    void attachPin(void);
    void detachPin(void);

    void step(void);

    void applyDirection(bool);
    // void applyVelocity(float);
    void increaseVelocity(void);

    // PWM signal controls
    void start(float, int);
};

#endif