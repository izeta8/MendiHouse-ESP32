#include <Arduino.h>  
#include <MFRC522.h>
#include "RFID.h"

//Constants
#define SS_PIN 5
#define RST_PIN 0

//Variables
byte nuidPICC[4] = {0, 0, 0, 0}; // NUID: Non-Unique IDentifier (IdCard)
MFRC522::MIFARE_Key key; // Used to encript.
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN); // The reader object.

// Functons definition.
void readRFID();
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);

// Initialize sensor.
void rfidBegin(void) {
  rfid.PCD_Init();
  Serial.print(F("Reader Version Info :"));
  rfid.PCD_DumpVersionToSerial();
}

void readRFID(void) { 

  // Read RFID card
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Look for new 1 cards
  if (!rfid.PICC_IsNewCardPresent()) return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial()) return;

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  Serial.print(F("RFID In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Stop PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


// Helper routine to dump a byte array as hex values to Serial.
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

// Helper routine to dump a byte array as dec values to Serial.
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}