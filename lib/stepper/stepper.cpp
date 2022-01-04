#include <Arduino.h>
#include <driver/ledc.h>
#include "stepper.h"

#define TIMER_I(i) (i == 0 ? LEDC_TIMER_0 : LEDC_TIMER_1)
#define CHANNEL_I(i) (i == 0 ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1)

// 20 * π / (200 * 32)
const float perstep = 20.0 * PI / (200.0 * 32.0);
                      // 0.009817477;

// Accelerate every tenth of a second
const float accelDelay = 0.1;

// Global turned on state of motors
bool motorState = false;
bool motorSleep = false;

// Pins to control state of motors
const int enPin = 12;
const int resPin = 33;
const int sleepPin = 32;
const int penPin = 22;

// Pins to set direction and move
// { leftPin, rightPin }
const int dirPins[2] = { 16, 18 };
const int stepPins[2] = { 17, 19 };

void setMotorSleep(bool on) {
  digitalWrite(sleepPin, !on);
  motorSleep = on;
}

// Turn the controlboard, motors on
void setMotorState(bool on) {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, !on);
  pinMode(resPin, OUTPUT);
  digitalWrite(resPin, on);
  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, on);
  motorState = on;
}

Motor::Motor(int index) {
  this->index = index;
}

// Function to accelerate the PWM signal
void accelCallback(void *_motor) {
  Motor *motor = (Motor *)_motor;
  ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(motor->index));
  ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(motor->index));
  int difference = esp_timer_get_time() - motor->timeLastCall;
  // Serial.printf("Difference: %d", difference);

  // Update steps done
  if (round(motor->velocity) >= 2) {
    // Serial.printf("Velocity: %f, difference: %d ...\n", motor->velocity, difference);
    int steps = ceil(round(motor->velocity) * difference/1000000.0);
    motor->stepsToDo -= steps;
    // Serial.printf("Steps done: %d, todo: %d\n", steps, motor->stepsToDo);
  }

  motor->velocity += motor->accel;
  int velocityRounded = round(motor->velocity);
  if (velocityRounded > round(motor->target_velocity)) {
    motor->velocity = motor->target_velocity;
    velocityRounded = round(motor->velocity);
  }

  if (velocityRounded < 2) {
    // Serial.println("Skipping ...");
    return;
  }

  // Serial.printf("Velocity (%d): %f\n", motor->index, motor->velocity);
  // Serial.printf("Applied acceleration: %f\n", appliedAcceleration); 
  // Serial.printf("Predicted steps: %d", predictedSteps);

  // Apply the velocity
  ledc_set_freq(LEDC_HIGH_SPEED_MODE, TIMER_I(motor->index), velocityRounded); 
  // dutyCycle should not be less than 2, so we always round up
  int dutyCycle = ceil((float)velocityRounded/500000.0 * 65535.0);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(motor->index), dutyCycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(motor->index)); 

  int predictedSteps = round(velocityRounded * accelDelay);

  // Start a stop timer if
  // -> all the steps will be executed before the next accel timer is called
  // -> or the plotter is fully accelerated
  if (motor->stepsToDo <= predictedSteps || velocityRounded == round(motor->target_velocity)) {
    // Calculate the remaining time
    int delay = round((float)(motor->stepsToDo)/(float)velocityRounded * 1000000.0);
    motor->stopDelay = delay;
    // Serial.printf("Delay: %d\n", delay);
    // motor->stepsToDo = 0;
    // Stop the acceleration timer and start the stop timer
    esp_timer_start_once(motor->stop_timer, delay);
    esp_timer_stop(motor->accel_timer);
  }
  // else {
  //   // Else calculate steps done until next callback 
  //   motor->stepsToDo -= predictedSteps;
  // }

  // Resume the PWM signal
  motor->timeLastCall = esp_timer_get_time();
  ledc_timer_resume(LEDC_HIGH_SPEED_MODE, TIMER_I(motor->index));
}

// Assign values
// Set up timer and PWM channel + timer
StepperMotor::StepperMotor(
  int index, int dirPin, int stepPin
) {
  // 0 or 1
  this->index = index;
  // Pins
  this->dirPin = dirPin;
  this->stepPin = stepPin;

  // Set the direction pin to OUTPUT
  pinMode(this->dirPin, OUTPUT);
  pinMode(this->stepPin, OUTPUT);

  // API documentation for `esp_timer`
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html

  // Create a new motor
  Motor *motor = new Motor(this->index);
  this->motor = motor;

  // Timer with callback to accelerate the pwm signal frequency
  const esp_timer_create_args_t accel_timer_args = {
    .callback = accelCallback,
    // Pass the motor with all the necessary
    // values and functions
    .arg = (void *)this->motor,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "Accel-Timer",
  };
  esp_timer_create(&accel_timer_args, &this->motor->accel_timer);

  // Timer with callback to stop the pwm signal
  const esp_timer_create_args_t stop_timer_args = {
    // Function to stop the PWM signal
    .callback = [](void *_motor){
      Motor *motor = (Motor *)_motor;
      ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(motor->index));
      ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(motor->index));
      int difference = esp_timer_get_time() - motor->timeLastCall;
      int steps = ceil(round(motor->velocity) * difference/1000000.0);
      motor->stepsToDo -= steps;
      Serial.printf("Steps: %d, todo (on %d): %d\n", steps, motor->index, motor->stepsToDo);
    },
    .arg = (void *)this->motor,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "Stop-Timer",
  };
  esp_timer_create(&stop_timer_args, &this->motor->stop_timer);

  // API documentation for `<driver/ledc.h>`
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html

  const ledc_timer_config_t pwm_timer_args = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    LEDC_TIMER_16_BIT,
    .timer_num = TIMER_I(this->index),
    1000,
  };
  ledc_timer_config(&pwm_timer_args);
  const ledc_channel_config_t pwm_channel_args = {
    .gpio_num = this->stepPin,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = CHANNEL_I(this->index),
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = TIMER_I(this->index),
    // Max duty for frequency of 1000Hz
    .duty = 16,
    // High point (0 = at the beginning)
    // .hpoint = 0x888888,
    .hpoint = 0x000000,
  };
  ledc_channel_config(&pwm_channel_args);

  this->attached = true;

  ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index));
  ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index));
}

// Connect the step pin to the PWM channel
void StepperMotor::attachPin() {
  if (!this->attached) {
    this->attached = true;
    ledcAttachPin(this->stepPin, CHANNEL_I(this->index));
  }
}

// Disconnect the step pin from the PWM channel 
void StepperMotor::detachPin() {
  if (this->attached) {
    this->attached = false;
    ledcDetachPin(this->stepPin);
  }
}

// Do one step
// Only works if the step pin is detached from the PWM channel
void StepperMotor::step() {
  if (!this->attached) {
    digitalWrite(this->stepPin, HIGH);
    delayMicroseconds(2);
    digitalWrite(this->stepPin, LOW);
  }
}

// Set the direction (counter-clockwise -> 0, clockwise -> 1)
void StepperMotor::applyDirection(bool shorter) {
  digitalWrite(this->dirPin, shorter ? (int)!(bool)this->index : this->index);
}

// Execute certain amount of steps with specific target velocity
// // TODO: If `acceleration` is true, the motor will accelerate to the target velocity
// // if not, the velocity will just be applied
void StepperMotor::doSteps(float t_velocity, int steps) {
  if (this->attached) {
    // If target velocity is less than 2, do the steps manually
    if (t_velocity < 1.5) {
      Serial.printf("Executing %d steps (velocity %f) manually ...\n", steps, t_velocity);
      this->detachPin();
      for (int i = 0; i < steps; i++) {
        this->step();
      }
      this->attachPin();
      return;
    }

    this->motor->velocity = 0;
    this->motor->stepsToDo = steps;
    this->motor->target_velocity = t_velocity;
    this->motor->accel = t_velocity/10.0;

    // Serial.printf("Accel: %f\n", this->motor->accel);

    // Start the accel timer (1s (10 x 0.1s))
    // The accel timer will automatically start
    // the stop timer after fully accelerating
    this->motor->timeLastCall = esp_timer_get_time();
    esp_timer_start_periodic(this->motor->accel_timer, 1000000 * accelDelay);
  }
}