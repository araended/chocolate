#ifndef ETHERNET_HANDLER_H
#define ETHERNET_HANDLER_H

#include <Arduino.h>
#include <SPI.h>
#include <ETH.h>
#include <WiFiUdp.h> // Native ESP32 UDP library (Works seamlessly with ETH.h)
#include "hardware_config.h" 

// thrust1, thrust2, thrust3, thrust4, thrust5, thrust6, depth, servo > total 8 commands
#define DATA_LENGTH 8
#define ETH_DATA_SIZE 8

// Function declarations
void init_ethernet();
bool send_ethernet_data(String data_to_be_sent);
bool receive_ethernet_data(int ethernet_data[DATA_LENGTH]);

#endif