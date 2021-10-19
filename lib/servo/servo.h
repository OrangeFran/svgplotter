#ifndef SERVO_H
#define SERVO_H

extern const int servoPin;

class Servo {
  public:
    int pin;
    bool down;

    // Constructor
    Servo(int);
    int penUp();
    int penDown();
    int toggle();
};

#endif