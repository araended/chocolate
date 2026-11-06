
#include <Arduino.h>
#include "hardware_config.h"
#include "servo_handler.h"
#include "thruster_control.h"
#include "ethernet_handler.h"
#include "sensor_handler.h"
#include "motor_driver.h"

// Shared global arrays
int thruster_cmd[NUM_THRUSTERS];
int ethernet_data[ETH_DATA_SIZE];

// Shared global variable
int servo_cmd = 1500;

// FreeRTOS Mutex Handle
SemaphoreHandle_t xDataMutex;

// Task prototypes
void vPIDControlTask(void *pvParameters);
void vCommunicationsTask(void *pvParameters);
void read_serial_data(int data[ETH_DATA_SIZE]);

void setup() {
    Serial.begin(115200);

    init_motor_driver();
    init_sensor();
    init_ethernet();
    init_servo();
    init_thruster();

    // Initialize command values
    for (int i = 0; i < NUM_THRUSTERS; i++) {
        thruster_cmd[i] = 1500;
    }

    for (int i = 0; i < ETH_DATA_SIZE; i++) {
        ethernet_data[i] = 1500;
    }

    servo_cmd = 1500;

    xDataMutex = xSemaphoreCreateMutex();

    if (xDataMutex != NULL) {
        xTaskCreatePinnedToCore(
            vPIDControlTask,
            "PID Control",
            4096,
            NULL,
            2,
            NULL,
            0
        );

        xTaskCreatePinnedToCore(
            vCommunicationsTask,
            "Communications",
            4096,
            NULL,
            1,
            NULL,
            1
        );
    } else {
        Serial.println("Failed to create Mutex!");
    }
}

void loop() {
    // FreeRTOS handles task scheduling
}

void vPIDControlTask(void *pvParameters) {
    int local_thruster_command[NUM_THRUSTERS];
    int local_servo_command = 1500;

    while (1) {
        if (xSemaphoreTake(xDataMutex, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < NUM_THRUSTERS; i++) {
                local_thruster_command[i] = thruster_cmd[i];
            }

            local_servo_command = servo_cmd;

            xSemaphoreGive(xDataMutex);
        }

        for (int i = 0; i < NUM_THRUSTERS; i++) {
            set_thruster(i, local_thruster_command[i]);
        }

        String msg = "";

        for (int i = 0; i < NUM_THRUSTERS; i++) {
            msg += String(local_thruster_command[i]);

            if (i < NUM_THRUSTERS - 1) {
                msg += ",";
            }
        }

        send_ethernet_data(msg);

        set_servo(local_servo_command);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vCommunicationsTask(void *pvParameters) {
    while (1) {
        read_serial_data(ethernet_data);

        if (receive_ethernet_data(ethernet_data)) {
            // Process received data
        }

        if (xSemaphoreTake(xDataMutex, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < NUM_THRUSTERS; i++) {
                thruster_cmd[i] = ethernet_data[i];
            }

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

void read_serial_data(int data[ETH_DATA_SIZE]) {
    if (Serial.available()) {
        for (int i = 0; i < ETH_DATA_SIZE; i++) {
            data[i] = Serial.parseInt();
        }

        while (Serial.available()) {
            Serial.read();
        }
    }
}
