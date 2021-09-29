extern float perstep;
extern bool motorState;
void setMotorState(bool);

extern int dirPins[2];
extern int stepPins[2];

class StepperMotor {
  public:
    // left motor -> 0
    // right motor -> 1
    int index; 
    // steps per second (= Hz)
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