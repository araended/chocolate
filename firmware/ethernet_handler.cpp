#include "ethernet_handler.h"

// --- Network Configuration ---
// Note: ETH.h requires a Gateway and Subnet when setting a static IP
IPAddress static_ip(192, 168, 42, 177); 
IPAddress gateway(192, 168, 42, 1);    
IPAddress subnet(255, 255, 255, 0);    

unsigned int localPort = 8888;  // The port the ROV will listen on

IPAddress remote_ip(192, 168, 42, 99); // Your topside computer's IP
unsigned int remote_port = 8888;       // Your topside computer's listening port

// Create the UDP instance
WiFiUDP Udp;

void init_ethernet() {
  // 1. Hardware Reset the W5500 chip 
  pinMode(PIN_W5500_RST, OUTPUT);
  digitalWrite(PIN_W5500_RST, LOW);
  delay(10);
  digitalWrite(PIN_W5500_RST, HIGH);
  delay(150); // Give it time to wake up

  // 2. Initialize the ESP32-S3 SPI bus
  // Notice we DO NOT pass the CS pin here for the S3 hardware SPI
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
  
  // 3. Start the ETH library
  // Parameters: phy_type, phy_address, cs_pin, irq_pin, rst_pin, spi_bus
  if (!ETH.begin(ETH_PHY_W5500, 1, PIN_W5500_CS, -1, PIN_W5500_RST, SPI)) {
    Serial.println("❌ ERROR: ETH.begin failed. Check SPI wiring.");
  } else {
    Serial.println("✅ ETH Driver Started Successfully.");
  }

  // 4. Force the Static IP configuration
  ETH.config(static_ip, gateway, subnet);
  
  // Small delay to let the network link negotiate
  delay(3000);
  
  if (ETH.linkUp()) {
    Serial.print("✅ Ethernet Link UP. IP: ");
    Serial.println(ETH.localIP());
  } else {
    Serial.println("⚠️ Ethernet Link DOWN (Is the tether unplugged?)");
  }
  delay(3000);

  // 5. Start listening for UDP packets
  Udp.begin(localPort);
}

bool send_ethernet_data(String data_to_be_sent) {
  // 1. Open a connection to the destination IP and Port
  Udp.beginPacket(remote_ip, remote_port);
  
  // 2. Load the string data into the packet
  Udp.print(data_to_be_sent);
  
  // 3. Fire the packet over the network! 
  if (Udp.endPacket() == 1) {
    Serial.print("Data sent to ethernet : ");
    Serial.print(data_to_be_sent);
    return true;  // Sent successfully
  } else {
    Serial.println("failed to send data through ethernet");
    return false; // Failed to send
  }
}

bool receive_ethernet_data(int ethernet_data[ETH_DATA_SIZE]) { 
  // 1. Check if any UDP packet has arrived
  int packetSize = Udp.parsePacket();
  
  if (packetSize > 0) {
    // 2. Create a character buffer to hold the incoming data
    char packetBuffer[256]; 
    
    // 3. Read the packet data into the buffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = '\0'; // Null-terminate
    }

    // 4. Parse the comma-separated string
    int index = 0;
    char* token = strtok(packetBuffer, ","); 
    
    while (token != NULL && index < ETH_DATA_SIZE) {
      ethernet_data[index] = atoi(token); 
      index++;
      token = strtok(NULL, ","); 
    }
    
    for(int i = 0; i < ETH_DATA_SIZE; i++){
          Serial.print("ethernet_data_received : ");
          Serial.print(ethernet_data[i]);
          Serial.print(" ");
        }
        Serial.println();

    return true; // Packet successfully received and parsed
  }
  
  return false; // No packet available to read during this loop cycle
}