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

bool Motor::finished() {
  // // API too new
  // int expiry_time;
  // if (!esp_timer_is_active(this->accel_timer)) {
  //   esp_timer_get_expiry_time(this->stop_timer, &expiry_time);
  //   if (expiry_time == 0) {
  //     return true;
  //   }
  // }
  // return false;

  // int next_alarm;
  // while (true) {
  //   next_alarm = esp_timer_get_next_alarm() - esp_timer_get_time();
  //   Serial.printf("Next alarm: %d", next_alarm);
  //   if (next_alarm < 0) {
  //     break;
  //   }
  // }
  return true;
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
      Serial.printf("Accelerating %d ...", stepper->index);

      // Increase velocity
      ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
      ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
      // Serial.printf("velocity: %f\n", stepper->velocity);
      // Serial.printf("accel: %f\n", stepper->accel);
      // float new_velocity = stepper->velocity + stepper->accel;
      stepper->increaseVelocity();

      Serial.printf("Velicty: %f\n", stepper->velocity);

      // Start a stop timer if
      // -> all the steps will be executed before the next accel timer is called
      // -> or the plotter is fully accelerated
      if (stepper->stepsToDo < (stepper->velocity * 0.2) || stepper->velocity == stepper->target_velocity) {
        // Calculate the remaining time
        int delay = (float)(stepper->stepsToDo)/(float)(stepper->velocity) * 1000000.0;
        stepper->stepsToDo = 0;
        // Stop the acceleration timer and start the stop timer
        esp_timer_stop(stepper->accel_timer);
        esp_timer_start_once(stepper->stop_timer, delay);
      } else {
        // Else calculate steps done until next callback 
        stepper->stepsToDo -= stepper->velocity * 0.2;
      }

      // Resume the PWM signal
      ledc_timer_resume(LEDC_HIGH_SPEED_MODE, TIMER_I(stepper->index));
    },
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

  // NOTE: Needed?
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

void Motor::increaseVelocity() {
  // if (this->attached) {
    // Velocity in sps
    this->velocity += this->accel;

    // Apply the velocity
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index), round(this->velocity)); 

    // The duty cycle does not have to be accurate to the point
    // Delay of 2 microseconds = freq of 500000Hz
    int dutyCycle = ceil((float)round(this->velocity)/500000.0 * 16383.0);
    // Use one if dutyCycle is too small
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(this->index), dutyCycle == 0 ? 1 : dutyCycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(this->index)); 
  // }
}

// void StepperMotor::setTargetVelocity(int target_velocity) {
//   this->target_velocity = target_velocity;
//   this->accel = this->target_velocity/10;
// }

// Make the string for a certain motor longer/shorter
void Motor::start(float t_velocity, int steps) {
  // if (this->attached) {
    this->velocity = 0;
    this->stepsToDo = steps;
    this->target_velocity = t_velocity;
    this->accel = this->target_velocity/10.0;

    // Start the accel timer
    // The accel timer will automatically start
    // the stop timer after fully accelerating
    esp_timer_start_periodic(this->accel_timer, 100000);
  // }
}

// // Stop the motor
// void StepperMotor::stop() {
//   if (this->attached) {
//     ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(index));
//     ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(index));
//   }
// }