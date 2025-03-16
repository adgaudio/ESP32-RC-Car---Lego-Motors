#include "lego_motor.h"
#include <Arduino.h>
#include <stdint.h>

void LegoDCMotor::init(uint8_t gpioA, uint8_t gpioB, uint32_t freq) {
  this->gpioA = gpioA;
  this->gpioB = gpioB;
  this->maxDutyCycle = (1 << resolution_bits) - 1;
  ledcAttach(gpioA, freq, resolution_bits);
  ledcAttach(gpioB, freq, resolution_bits);
  // Default to forward direction
  move(0, 0);
}

// Set motor speed (duty cycle)
void LegoDCMotor::move(uint8_t speed_8bit, bool dir) const {
  // speed: value in [0, 255]
  // dir: value is 0 or 1
  if (dir == 0) {
    ledcWrite(gpioA, 0);
    ledcWrite(gpioB, speed_8bit);
  } else {
    ledcWrite(gpioA, speed_8bit);
    ledcWrite(gpioB, 0);
  }
}
