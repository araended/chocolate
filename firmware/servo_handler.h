#ifndef SERVO_HANDLER_H
#define SERVO_HANDLER_H

#include <ESP32Servo.h>
#include "hardware_config.h"

void init_servo();
void set_servo(int value);

#endif