#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

bool init_motor_driver();
void stop_motors();
void set_motor_a(int direction);
void set_motor_b(int direction);

#endif
