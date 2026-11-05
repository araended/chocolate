#include "servo_handler.h"

Servo servo;

void init_servo(){
  servo.attach(SERVO_PIN);
}

void set_servo(int angle){
  if (angle < 0){
    angle = 0;
  } else if (angle > 180){
    angle = 180;
  }
 // Write angle directly
    servo.write(angle);
    Serial.print("Servo has been set to: ");
    Serial.println(angle);
}