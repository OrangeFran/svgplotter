#include <Arduino.h>

float perstep = 0.019625; // 0.07925;

// Pins to control state of motors
int enPin = 12;
int resPin = 33;
int sleepPin = 32;

// Global turned on state of motors
bool motorState = false;

// Turn the motors on
void setMotorState(bool on) {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, !on);
  pinMode(resPin, OUTPUT);
  digitalWrite(resPin, on);
  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, on);
  motorState = on;
}

class StepperMotor {
  private:
    int index; // left -> 0, right -> 1
    float velocity;
    int dirPin;
    int stepPin;

  public:
    // Constructor
    // Set the pins to OUTPUT
    StepperMotor(int index, int dirPin, int stepPin) {
      index = index;
      dirPin = dirPin;
      stepPin = stepPin;
    }

    void setVelocity(float newVelocity) {
      velocity = newVelocity;
    }

    // Do one step
    void step() {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(2);
      digitalWrite(stepPin, LOW);
    }

    // Make the string for a certain motor longer/shorter
    int travel(int distance) {
      int stepper = this->index;
      float velocity = this->velocity;
    
      // Set the direction
      digitalWrite(this->dirPin, distance > 0 ? stepper : (int)!(bool)stepper);

      // Calculate and do steps
      // The direction is already set, so the prefix can be
      // removed with abs()
      int steps = abs(distance)/perstep;
      TickType_t x_last_wake_time = xTaskGetTickCount();
      for (int i = 0; i < steps; i++) {
        this->step();
        // delayMicroseconds will not work,
        // because it does not provide time for the watchdog
        // vTaskDelay is absolute, vTaskDelayUntil is relative
        vTaskDelayUntil(&x_last_wake_time, velocity * portTICK_PERIOD_MS);
        // vTaskDelay(velocity * portTICK_PERIOD_MS);
      }
    
      return 0;
    }
};
