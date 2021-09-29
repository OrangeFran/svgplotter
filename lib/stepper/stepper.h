extern float perstep;
extern bool motorState;
void setMotorState(bool);

extern int dirPins[2];
extern int stepPins[2];

class StepperMotor {
  public:
    int index; // left -> 0, right -> 1
    float velocity; // sps
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