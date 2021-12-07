#include <Arduino.h>
#include <driver/ledc.h>
#include "stepper.h"

#define TIMER_I(i) (i == 0 ? LEDC_TIMER_0 : LEDC_TIMER_1)
#define CHANNEL_I(i) (i == 0 ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1)

// 20 * π / (200 * 32)
const float perstep = 0.009817477;

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
    // Function to stop the PWM signal
    .callback = [](void *_stepper){
      Motor *stepper = (Motor *)_stepper;
      ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
      ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
      // Increase velocity
      // Serial.println("Increasing velocity");
      // Serial.printf("Steps to do (-1): %d", stepper->stepsToDo);
      // stepper->increaseVelocity();

      // Velocity in sps
      stepper->velocity += stepper->accel;
      // If no step is done, don't bother
      if (round(stepper->velocity * 0.1) < 1) {
        Serial.printf("Skipping with velocity: %f\n", stepper->velocity);
        return;
      }

      // Serial.printf("Velo: %f\n", stepper->velocity);
      // Serial.printf("Rounded velo: %d\n", (int)round(stepper->velocity));

      // Apply the velocity
      ledc_set_freq(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index), (int)round(stepper->velocity)); 
      // if (error == ESP_ERR_INVALID_ARG) {
      //   Serial.println("Invalid arg!");
      // } else if (error == ESP_FAIL) {
      //   Serial.printf("Fail");
      // }

      // The duty cycle does not have to be accurate to the point
      // Delay of 2 microseconds = freq of 500000Hz
      // Serial.println("Calculating duty ...");
      int dutyCycle = round((float)round(stepper->velocity)/500000.0 * 16383.0);
      // Serial.printf("DT: %d", dutyCycle);
      // Use one if dutyCycle is too small
      ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(stepper->index), dutyCycle == 0 ? 1 : dutyCycle);
      ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(stepper->index)); 

      // Start a stop timer if
      // -> all the steps will be executed before the next accel timer is called
      // -> or the plotter is fully accelerated
      if (stepper->stepsToDo <= round(stepper->velocity * 0.1) || round(stepper->velocity) == round(stepper->target_velocity)) {
        // Calculate the remaining time
        // Serial.printf("Steps to do (0): %d", stepper->stepsToDo);
        // Serial.printf("Velocity (0): %f", stepper->velocity);
        int delay = round((float)(stepper->stepsToDo)/(float)(stepper->velocity) * 1000000.0);
        // Serial.printf("Steps to do: %d", stepper->stepsToDo);
        stepper->stepsToDo = 0;
        // Stop the acceleration timer and start the stop timer
        esp_timer_stop(stepper->accel_timer);
        esp_timer_start_once(stepper->stop_timer, delay);
        // Serial.println("Started timer!");
      } else {
        // Else calculate steps done until next callback 
        stepper->stepsToDo -= (int)round(stepper->velocity * 0.1);
      }
      // Resume the PWM signal
      ledc_timer_resume(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
    },
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
    .callback = [](void *index){
      ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(*(int *)index));
    },
    .arg = &this->index,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "Stop-Timer",
  };
  esp_timer_create(&stop_timer_args, &this->motor->stop_timer);

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
    // High point (0 = at the beginning)
    .hpoint = 0,
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

// Requires pins to be attached
  // to the PWM signal
void Motor::increaseVelocity() {
  // Velocity in sps
  this->velocity += this->accel;

  if (round(this->velocity * 0.1) < 1) {
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index), 0); 
    return;
  }

  Serial.printf("Velo: %f\n", this->velocity);
  Serial.printf("Rounded velo: %d\n", (int)round(this->velocity));

  // Apply the velocity
  ledc_set_freq(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index), (int)round(this->velocity)); 
  // if (error == ESP_ERR_INVALID_ARG) {
  //   Serial.println("Invalid arg!");
  // } else if (error == ESP_FAIL) {
  //   Serial.printf("Fail");
  // }

  // The duty cycle does not have to be accurate to the point
  // Delay of 2 microseconds = freq of 500000Hz
  Serial.println("Calculating duty ...");
  int dutyCycle = round((float)round(this->velocity)/500000.0 * 16383.0);
  Serial.printf("DT: %d", dutyCycle);
  // Use one if dutyCycle is too small
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(this->index), dutyCycle == 0 ? 1 : dutyCycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(this->index)); 
}

// void StepperMotor::setTargetVelocity(int target_velocity) {
//   this->target_velocity = target_velocity;
//   this->accel = this->target_velocity/10;
// }

// Make the string for a certain motor longer/shorter
void StepperMotor::doSteps(float t_velocity, int steps) {
  if (this->attached) {
    this->motor->velocity = 0;
    this->motor->stepsToDo = steps;
    this->motor->target_velocity = t_velocity;
    this->motor->accel = t_velocity/10.0;

    // Start the accel timer (1s (10 x 0.1s))
    // The accel timer will automatically start
    // the stop timer after fully accelerating
    esp_timer_start_periodic(this->motor->accel_timer, 100000);
  }
}

// // Stop the motor
// void StepperMotor::stop() {
//   if (this->attached) {
//     ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(index));
//     ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(index));
//   }
// }