#ifndef LEGO_MOTOR_H
#define LEGO_MOTOR_H

#include <stdint.h>

// Motor control class for modularity
class LegoDCMotor {
private:
  uint8_t gpioA, gpioB; // GPIO pins for forward and reverse
  uint8_t resolution_bits = 8;
  int maxDutyCycle;

public:
  void init(uint8_t pinA, uint8_t pinB, uint32_t freq = 1200);
  void move(uint8_t speed_8bit, bool dir) const;
};
#endif // LEGO_MOTOR_H
