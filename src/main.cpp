#include <Arduino.h>  
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <RFID/RFID.h>

void setup() {

  // Init Serial USB
  Serial.begin(115200);
  SPI.begin();
  rfidBegin();
  
}

void loop() {
  readRFID();
}
