#include <Arduino.h>
#include <driver/ledc.h>
#include "stepper.h"

#define TIMER_I(i) (i == 0 ? LEDC_TIMER_0 : LEDC_TIMER_1)
#define CHANNEL_I(i) (i == 0 ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1)

// TODO: measure/calculate exact mm per step
// Current number based on trial and error
const float perstep = 0.01;
// Global turned on state of motors
bool motorState = false;

// Pins to control state of motors
const int enPin = 12;
const int resPin = 33;
const int sleepPin = 32;
const int penPin = 22;

// Pins to set direction and move
// { leftPin, rightPin }
const int dirPins[2] = { 16, 18 };
const int stepPins[2] = { 17, 19 };

const int initialVelocity = 200;

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

// Old function still needed for lib/joystick
// Do one step with the specified motor
void step(int stepper) {
  digitalWrite(stepPins[stepper], HIGH);
  delayMicroseconds(2);
  digitalWrite(stepPins[stepper], LOW);
}

// Constructor
StepperMotor::StepperMotor(
  int index, int dirPin, int stepPin
) {
  // 0 or 1
  this->index = index;

  this->dirPin = dirPin;
  this->stepPin = stepPin;
  pinMode(this->dirPin, OUTPUT);
  pinMode(this->stepPin, OUTPUT);

  // API documentation for `esp_timer`
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html

  // Timer with callback to accelerate the pwm signal frequency
  const esp_timer_create_args_t accel_timer_args = {
    // Function to stop the PWM signal
    .callback = [](void *_stepper){
      StepperMotor *stepper = (StepperMotor *)_stepper;
      // Calculate steps done
      stepper->stepsToDo -= stepper->current_velocity * 0.2;
      // Increase velocity
      ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
      ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
      stepper->setVelocity(stepper->current_velocity + stepper->accel);
      // Start a stop timer if all the steps will be executed
      // before the next accel timer is called
      if (stepper->stepsToDo > (stepper->current_velocity * 0.2)) {
        int delay = (float)(stepper->stepsToDo)/(float)(stepper->current_velocity) * 1000000.0;
        esp_timer_start_once(stepper->stop_timer, delay);
        ledc_timer_resume(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
      }
      // if (stepper->current_velocity < (stepper->target_velocity - stepper->accel)) {
      //   esp_timer_stop(stepper->accel_timer);
      // }
      if (stepper->current_velocity == stepper->target_velocity) {
        esp_timer_stop(stepper->accel_timer);
      }
    },
    .arg = this,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "Accel-Timer",
  };
  esp_timer_create(&accel_timer_args, &this->accel_timer);

  // Timer with callback to stop the pwm signal
  const esp_timer_create_args_t stop_timer_args = {
    // Function to stop the PWM signal
    .callback = [](void *index){
      ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(*(int *)index));
    },
    .arg = &this->index,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "Stop-Timer",
  };
  esp_timer_create(&stop_timer_args, &this->stop_timer);

  // API documentation for `<driver/ledc.h>`
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html

  const ledc_timer_config_t pwm_timer_args = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    LEDC_TIMER_14_BIT,
    .timer_num = TIMER_I(this->index),
    2000,
  };
  ledc_timer_config(&pwm_timer_args);

  const ledc_channel_config_t pwm_channel_args = {
    .gpio_num = this->stepPin,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = CHANNEL_I(this->index),
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = TIMER_I(this->index),
    // Max duty for frequency of 2000Hz
    .duty = 14,
    // High point (0 -> at the beginning)
    .hpoint = 0,
  };
  ledc_channel_config(&pwm_channel_args);

  // NOTE: Needed?
  ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index));
  ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index));
}

int StepperMotor::setAcceleration(int start_velocity, int target_velocity, bool shorter) {
  // Set the direction
  //  counter-clockwise -> 0, clockwise -> 1
  digitalWrite(this->dirPin, shorter ? (int)!(bool)this->index : this->index);

  this->start_velocity = start_velocity;
  this->target_velocity = target_velocity;
  // The motor is going to accelerate through ten levels
  this->accel = (target_velocity - start_velocity)/10;
  return 0;
}

void StepperMotor::setVelocity(
  int new_velocity
) {
  // Change the frequency accordingly
  ledc_set_freq(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index), (int)new_velocity); 

  // The duty cycle does not have to be accurate to the point
  // Delay of 2 microseconds = freq of 500000Hz
  int dutyCycle = ceil((float)new_velocity/500000.0 * 16383.0);
  // Use one if dutyCycle is too small
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(this->index), dutyCycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(this->index)); 

  // Velocity in sps
  this->current_velocity = new_velocity;
}

// Do one step
void StepperMotor::step() {
  digitalWrite(this->stepPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(this->stepPin, LOW);
}

// Make the string for a certain motor longer/shorter
int StepperMotor::start(int steps) {
  this->stepsToDo = steps;

  // Apply starting speed
  ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index));
  ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index));
  this->setVelocity(this->start_velocity);
  ledc_timer_resume(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index));

  // Start the accel timer
  esp_timer_start_periodic(this->accel_timer, 0.2);
  return 0;
}