#include <Arduino.h>
#include <driver/ledc.h>

#include "stepper.h"

#define TIMER_I(i) (i == 0 ? LEDC_TIMER_0 : LEDC_TIMER_1)
#define CHANNEL_I(i) (i == 0 ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1)

// #define CONFIG_FREERTOS_HZ 1000

// TODO: measure/calculate exact mm per step
// Current number based on trial and error
float perstep = 0.01;
                // 0.009817477;
                // 0.019625;
                // 0.07925;

// Pins to set direction and move: { leftPin, rightPin }
int dirPins[2] = {16, 18};
int stepPins[2] = {17, 19};

// Pins to control state of motors
int enPin = 12;
int resPin = 33;
int sleepPin = 32;

// Global turned on state of motors
bool motorState = false;

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
StepperMotor::StepperMotor(int index, int dirPin, int stepPin) {
  // 0 / 1
  this->index = index;
  // Pins
  this->dirPin = dirPin;
  this->stepPin = stepPin;

  // Set the direction pin to OUTPUT
  pinMode(this->dirPin, OUTPUT);
  pinMode(this->stepPin, OUTPUT);

  // Docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html

  // Timer with callback to stop the pwm signal
  const esp_timer_create_args_t stop_timer_args = {
    // Function to stop the PWM signal
    .callback = [](void *index){
      // ledcWrite(*((int*)index) * 2, 0);
      ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(*(int *)index));
      // ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(index));
      // Serial.printf("Reset timer %d on channel %d (%d)!\n", TIMER_I(*(int *)index), CHANNEL_I(*(int *)index), *(int *)index);
    },
    .arg = &this->index,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "PWM-Stop-Timer",
  };
  esp_timer_create(&stop_timer_args, &this->timer);

  // Docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html

  // Attach a channel to the stepping pin 
  // There are only 8 timers for 16 channels
  // -> subsequent channels use the same timer
  // and thus cannot be driven by different frequencies
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

  // // Attach a channel to the stepping pin 
  // // There are only 8 timers for 16 channels
  // // -> subsequent channels use the same timer
  // // and thus cannot be driven by different frequencies
  // ledcAttachPin(this->stepPin, this->index * 2);
  // // Frequency is arbitrary on init and will be set
  // // with the velocity
  // ledcSetup(this->index * 2, 1000, 14);
}

void StepperMotor::setVelocity(int velocity, bool shorter) {
  // Velocity in sps
  this->velocity = velocity;

  // Set the direction
  digitalWrite(this->dirPin, shorter ? (int)!(bool)this->index : this->index);
  // // Velocity specifies the delay in microseconds
  // ledcWriteTone(this->index * 2, (int)this->velocity);

  // Velocity specifies the delay in microseconds
  ledc_set_freq(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index), (int)this->velocity); 

  // The dutyCycle does not have to be accurate to the point
  // Delay of 2 microseconds = freq of 500000Hz
  int dutyCycle = ceil((float)this->velocity/500000.0 * 16383.0);
  // Use one if dutyCycle is too small
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(this->index), dutyCycle == 0 ? 1 : dutyCycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNEL_I(this->index)); 

  // Change the resolution
  // This is necessary because the available resolution
  // reduces if the frequency increases
  //
  // Max resolution possible for f: f(f) = log_2(80MHz/f)
  //   for f = 1 -> ~26
  //
  // Formulae:
  //   (f/500000) * (2**x - 1) = 1
  //   => x = 1.4427 * ln(1 + 500000/f)
  //
  // Delay of 2 microseconds = freq of 500000Hz
  // int resolution = round(1.4427 * log(1.0 + (500000.0/this->velocity)));
}

// Do one step
void StepperMotor::step() {
  digitalWrite(this->stepPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(this->stepPin, LOW);
}

// Make the string for a certain motor longer/shorter
int StepperMotor::start(int delay) {
  // // Calculate and do steps
  // // The direction is already set, so the prefix can be
  // // removed with abs()
  // int steps = abs(distance)/perstep;

  // // With current frequency: One tick = one millisecond
  // // TickType_t x_last_wake_time = xTaskGetTickCount();
  // // TickType_t frequency = velocity * portTICK_PERIOD_MS;
  // for (int i = 0; i < steps; i++) {
  //   this->step();
  //   // delayMicroseconds will not work,
  //   // because it does not provide time for the watchdog
  //   // vTaskDelay is absolute, vTaskDelayUntil is relative
  //   vTaskDelayUntil(&x_last_wake_time, frequency);
  // }

  // // The dutyCycle does not have to be accurate to the point
  // // Delay of 2 microseconds = freq of 500000Hz
  // int dutyCycle = round((float)this->velocity/500000.0 * 16383.0);
  // esp_timer_start_once(this->timer, delay);
  // // Use one if dutyCycle is too small
  // ledcWrite(this->index * 2, dutyCycle == 0 ? 1 : dutyCycle);

  esp_timer_start_once(this->timer, delay);
  ledc_timer_resume(LEDC_HIGH_SPEED_MODE, TIMER_I(this->index));
  return 0;
}

// Stop the motor (= set dutyCycle to 0)
int StepperMotor::stop() {
  ledc_timer_pause(LEDC_HIGH_SPEED_MODE, TIMER_I(index));
  ledc_timer_rst(LEDC_HIGH_SPEED_MODE, TIMER_I(index));
  // ledcWrite(this->index * 2, 0);
  return 0;
}