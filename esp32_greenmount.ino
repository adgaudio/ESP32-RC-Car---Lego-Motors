#include "lego_motor.h"
#include "wifi_webserver.h"

LegoDCMotor left_motor, right_motor;

void setup() {
    // Configure each motor
    left_motor.init(15, 2);
    right_motor.init(0, 4);
    WiFi_WebServer::init(left_motor, right_motor);

    // enable the h bridge
    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);
}

void loop() {
    WiFi_WebServer::handleClient();
  //   for (int d=0 ; d<255 ; d++) {
  // left_motor.move(d, true);
  // right_motor.move(d, true);
  // delay(10);
  //   }
  //   for (int d=0 ; d<255 ; d++) {
  // left_motor.move(d, false);
  // right_motor.move(d, false);
  // delay(10);
  //   }

  // left_motor.move(128, 1);
  // right_motor.move(0, 0);
  // delay(10);
  // Motor control works.  Servo doesn't!
  // if (dir == HIGH) {
  //   dir = LOW;
  // } else {
  //   dir = HIGH;
  // }
  // // digitalWrite(15, dir);
  // // turn one way
  // digitalWrite(15, HIGH);
  // digitalWrite(2, LOW);
  // delay(500);
  // // turn the other way
  // digitalWrite(15, LOW);
  // digitalWrite(2, HIGH);
  // // digitalWrite(2, LOW);
  // delay(500);
  // // hard stop, holding torque
  // digitalWrite(15, HIGH);
  // digitalWrite(2, HIGH);
  // delay(100);
  // // loose stop, no torque
  // digitalWrite(15, LOW);
  // digitalWrite(2, LOW);
  // delay(400);

// servo - ours has only 3 positoins, so pwm is not needed.
    // if (servo_dir == 0) {
    // digitalWrite(4, LOW);
    // digitalWrite(16, HIGH);
    // } else if (servo_dir == 2) {
    // digitalWrite(4, HIGH);
    // digitalWrite(16, LOW);
    // } else if (servo_dir == 1) {
    // digitalWrite(4, LOW);
    // digitalWrite(16, LOW);
    // }
    // servo_dir = (servo_dir + 1) %3;
    // delay(500);

}
