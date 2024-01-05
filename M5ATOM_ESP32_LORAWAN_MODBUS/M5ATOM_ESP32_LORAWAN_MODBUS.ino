#include <Arduino.h>
#include <M5Atom.h>
// #include <ArduinoModbus.h>
// #include <ArduinoRS485.h>
#include "ATOM_DTU_LoRaWAN.h"

#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>

#define RX2 16
#define TX2 17
#define RE_PIN 2   // You can change this pin as per your setup
#define DE_PIN 2   // You can change this pin as per your setup

HardwareSerial RS485Serial(2);
SoftwareSerial rs485(2, 3); // RX, TX
ModbusMaster node;

void setup() {
  rs485.begin(9600);
  node.begin(1, rs485); // Set Modbus slave ID to 1
}

void loop() {
  uint8_t result;
  uint16_t data;

  // Read a holding register (address 0x0001) from the Modbus device
  result = node.readHoldingRegisters(0x0001, 1); // Address and number of registers to read

  if (result == node.ku8MBSuccess) {
    // Read was successful, get the data
    data = node.getResponseBuffer(0); // Get the data from the response buffer
    Serial.print("Read successful. Data: ");
    Serial.println(data);
  } else {
    // Read failed
    Serial.println("Read failed.");
  }

  delay(1000); // Delay before the next read
}


// void setup() {
//   // put your setup code here, to run once:
//   // HardwareSerial.begin(true, false, true);

//   pinMode(RE_PIN, OUTPUT);
//   pinMode(DE_PIN, OUTPUT);
//   RS485Serial.begin(9600, SERIAL_8N1, RX2, TX2);
//   node.begin(1, RS485Serial); // Set Modbus slave ID to 1
//   // node.preTransmission(preTransmission);
//   // node.postTransmission(postTransmission);

//   // Serial.println("Hello, ESP32!");
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   delay(10); // this speeds up the simulation
  
// }

