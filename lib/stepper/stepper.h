extern int motorState;
void setMotorState(bool);
class StepperMotor {
  public:
    // Constructor
    StepperMotor(int, int, int);
    void setVelocity(float);
    void step();
    int travel(int distance);
};
