#ifndef THRUSTER_CONTROL_H
#define THRUSTER_CONTROL_H

#include <ESP32Servo.h>
#include "hardware_config.h" // Ensures NUM_THRUSTERS and PWM_FREQ are known
#include "ethernet_handler.h"

// Declare the servo array as 'extern' so main.ino can see and control it later
extern Servo thruster[NUM_THRUSTERS];

// #define MAXTHRUST 2000;
// #define MINTHRUST 1000;

// Function declaration
void init_thruster();
void set_thruster(int index, int microseconds);

#endif