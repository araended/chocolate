#include "thruster_control.h"

Servo thruster[NUM_THRUSTERS];

void init_thruster() {
  int thruster_pins[NUM_THRUSTERS] = {PIN_THRUSTER_1, PIN_THRUSTER_2, PIN_THRUSTER_3, PIN_THRUSTER_4, PIN_THRUSTER_5, PIN_THRUSTER_6};

  for (int i = 0; i < NUM_THRUSTERS; i++) {
    thruster[i].setPeriodHertz(PWM_FREQ);
    thruster[i].attach(thruster_pins[i], 1000, 2000); 
    
    // Safety: Send neutral signal (1500us) immediately on boot so ESCs can arm
    // FORCE a 1500us loop for 3-5 seconds so the ESCs can successfully arm
    Serial.println("Arming ESCs... Keep clear.");
    send_ethernet_data("Arming ESC......");
    for (int seconds = 0; seconds < NUM_THRUSTERS; seconds++) {
      for (int i = 0; i < 4; i++) {
        thruster[i].writeMicroseconds(1500);
      }
      delay(1000); // Standard block delay is fine here because scheduler hasn't started
    }
    Serial.println("ESCs Armed!");
  }
}

// UPDATE: Change this to take the specific thruster index and a single microsecond value
void set_thruster(int index, int microseconds) {
  // Guard clause against invalid array indexing
  if (index < 0 || index >= NUM_THRUSTERS) return; 

  // Constrain the value safely using local variables (doesn't modify global state)
  if (microseconds > 2000) {
    microseconds = 2000;
  } else if (microseconds < 1000) {
    microseconds = 1000;
  }
  
  // Write directly to the target hardware instance
  thruster[index].writeMicroseconds(microseconds);

  
}