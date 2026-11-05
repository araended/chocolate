#include "hardware_config.h"
#include "servo_handler.h"
#include "thruster_control.h"
#include "ethernet_handler.h"
#include "sensor_handler.h"

// Shared global arrays
int thruster_cmd[NUM_THRUSTERS]; 
int ethernet_data[ETH_DATA_SIZE]; 

// Shared global variable
int servo_cmd;

// FreeRTOS Mutex Handle to guard the shared arrays
SemaphoreHandle_t xDataMutex;

// Task Prototypes
void vPIDControlTask(void *pvParameters);
void vCommunicationsTask(void *pvParameters);

void setup() {
  Serial.begin(115200);
  
  init_sensor();
  init_ethernet();
  init_servo();
  init_thruster();
  for (int i = 0; i < NUM_THRUSTERS; i++) {
      ethernet_data[i] = 1500;
      thruster_cmd[i] = 1500;
    }
  // Create the Mutex before starting tasks
  xDataMutex = xSemaphoreCreateMutex();

  if (xDataMutex != NULL) {
    // Create PID Control Task (Higher priority for time-critical stability loops)
    xTaskCreatePinnedToCore(
      vPIDControlTask,     // Task function
      "PID Control",       // Task name
      4096,                // Stack size (bytes/words depending on platform)
      NULL,                // Parameter passed
      2,                   // Priority (Higher)
      NULL,                // Task handle
      0                    // Core 0
    );

    // Create Communications Task
    xTaskCreatePinnedToCore(
      vCommunicationsTask,
      "Communications",
      4096,
      NULL,
      1,                   // Priority (Lower)
      NULL,                // Task handle
      1                    // Core 1 (Assuming dual-core like ESP32 to split load)
    );
  } else {
    Serial.println("Failed to create Mutex!");
  }
}

void loop() {
  // Empty. FreeRTOS handles task scheduling.
}

void vPIDControlTask(void *pvParameters) {
  int local_thruster_command[NUM_THRUSTERS];
  int local_servo_command;
  
  while (1) {
    // 1. Safely copy global data to local variables
    if (xSemaphoreTake(xDataMutex, portMAX_DELAY) == pdTRUE) {
      for(int i = 0; i < NUM_THRUSTERS; i++){
        local_thruster_command[i] = thruster_cmd[i];
      }
      local_servo_command = servo_cmd;
      xSemaphoreGive(xDataMutex);
    }

    // 2. Execute hardware control
    for (int i = 0; i < NUM_THRUSTERS; i++) {
      set_thruster(i, local_thruster_command[i]);
    }

    // 3. Print the status cleanly (Fixed variable names and array bounds)
    Serial.print("thrusters have been set to: ");
    for(int j = 0; j < NUM_THRUSTERS; j++){ // Used 'j' and limited to NUM_THRUSTERS
      Serial.print(local_thruster_command[j]);
      Serial.print(" ");
    }
    Serial.println();

    // 4. Send local_thruster_command to send_ethernet_data(string string); to show that it has been set, but the problem is the local_thruster_command is a array
    String msg = "";

    for (int i = 0; i < NUM_THRUSTERS; i++) {
        msg += String(local_thruster_command[i]);

        if (i < NUM_THRUSTERS - 1) {
            msg += ",";
        }
    }

    send_ethernet_data(msg);
    
    // set servo
    set_servo(local_servo_command);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void vCommunicationsTask(void *pvParameters) {
  while (1) {
    read_serial_data(ethernet_data);

    if (receive_ethernet_data(ethernet_data)){

    }

    if (xSemaphoreTake(xDataMutex, portMAX_DELAY) == pdTRUE) {
      // FIXED: Changed '<=' to '<' to avoid memory corruption
      for (int i = 0; i < NUM_THRUSTERS; i++) { 
        thruster_cmd[i] = ethernet_data[i];
      }

      // Index 7 is the 8th item (since index 6 is skipped as you mentioned)
      servo_cmd = ethernet_data[7]; 
      
      xSemaphoreGive(xDataMutex);
    }

    // Show the received raw data packet
    Serial.print("Received Data: ");
    for(int i = 0; i < ETH_DATA_SIZE; i++){
      Serial.print(ethernet_data[i]);
      Serial.print(" ");
    }
    Serial.println();

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}


void read_serial_data(int data[]) {
    if (Serial.available()) {
        for (int i = 0; i < ETH_DATA_SIZE; i++) {
            data[i] = Serial.parseInt();
        }

        // Clear remaining characters
        while (Serial.available()) {
            Serial.read();
        }
    }
}
