extern float perstep;
extern bool motorState;
void setMotorState(bool);

extern int dirPins[2];
extern int stepPins[2];

class StepperMotor {
  private:
    int index; // left -> 0, right -> 1
    float velocity;
    int dirPin;
  public:
    int stepPin;
    // Constructor
    StepperMotor(int, int, int);
    void step(void);
    void setVelocity(float);
    int travel(int distance);
    int stop(void);
};

// Old function still needed for lib/joystick
void step(int);
