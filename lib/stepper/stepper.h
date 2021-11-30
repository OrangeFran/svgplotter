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

class Motor {
  public:
    esp_timer_handle_t accel_timer;
    esp_timer_handle_t stop_timer;

    int stepsToDo;
    float target_velocity;

    int index; // Left motor -> 0, right motor -> 1
    float velocity; // Steps per second (= Hz)
    float accel; 

    Motor(int);
    void increaseVelocity(void);
    bool finished(void);
    // PWM signal controls
    void start(float, int);
};

class StepperMotor {
  public:
    int dirPin;
    int stepPin;
    // Specify if step pin is attached to PWM channel or not
    // Joystick only works if attached is false
    bool attached;

    int index; // Left motor -> 0, right motor -> 1
    Motor *motor;

    StepperMotor(int, int, int);

    void attachPin(void);
    void detachPin(void);

    void step(void);

    void applyDirection(bool);
    // void applyVelocity(float);
    void increaseVelocity(void);
};

#endif