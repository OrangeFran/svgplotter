extern bool motorState;
void setMotorState(bool);

extern int dirPins[2];
extern int stepPins[2];

class StepperMotor {
  private:
    int index; // left -> 0, right -> 1
    float velocity;
    int dirPin;
    int stepPin;
  public:
    // Constructor
    StepperMotor(int, int, int);
    void step();
    void setVelocity(float);
    int travel(int distance);
};

// Old function still needed for lib/joystick
void step(int);
