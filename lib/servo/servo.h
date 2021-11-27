#ifndef SERVO_H
#define SERVO_H

extern const int servoPin;

// Servo class to "control" the pen
class Servo {
  private:
    int pin;
    bool down;

  public:
    Servo(int);
    int penUp();
    int penDown();
    int toggle();
};

#endif