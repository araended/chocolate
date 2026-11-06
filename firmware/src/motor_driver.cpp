#include "motor_driver.h"
#include "hardware_config.h"

static bool motor_driver_ready = false;

static void set_motor_speed(int in1, int in2, int enable_pin, int direction) {
  if (!motor_driver_ready) {
    return;
  }

  if (direction > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (direction < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  ledcWrite(enable_pin, direction == 0 ? 0 : MOTOR_PWM_DUTY);
}

bool init_motor_driver() {
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_A_EN, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  pinMode(MOTOR_B_EN, OUTPUT);

  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  digitalWrite(MOTOR_A_EN, LOW);
  digitalWrite(MOTOR_B_IN1, LOW);
  digitalWrite(MOTOR_B_IN2, LOW);
  digitalWrite(MOTOR_B_EN, LOW);

  const bool motor_a_pwm_ready =
      ledcAttach(MOTOR_A_EN, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
  const bool motor_b_pwm_ready =
      ledcAttach(MOTOR_B_EN, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);

  if (!motor_a_pwm_ready || !motor_b_pwm_ready) {
    Serial.println("L298N PWM initialization failed; motors remain stopped.");
    if (motor_a_pwm_ready) {
      ledcWrite(MOTOR_A_EN, 0);
    }
    if (motor_b_pwm_ready) {
      ledcWrite(MOTOR_B_EN, 0);
    }
    return false;
  }

  motor_driver_ready = true;
  stop_motors();
  return true;
}

void stop_motors() {
  if (!motor_driver_ready) {
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_A_EN, LOW);
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, LOW);
    digitalWrite(MOTOR_B_EN, LOW);
    return;
  }

  set_motor_speed(MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_A_EN, 0);
  set_motor_speed(MOTOR_B_IN1, MOTOR_B_IN2, MOTOR_B_EN, 0);
}

void set_motor_a(int direction) {
  set_motor_speed(MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_A_EN, direction);
}

void set_motor_b(int direction) {
  set_motor_speed(MOTOR_B_IN1, MOTOR_B_IN2, MOTOR_B_EN, direction);
}
