#include <Arduino.h>  

#ifndef RFID_H
#define RFID_H

void readRFID();
void rfidBegin();
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);

#endif


// // rfidHandler.h
// #ifndef RFIDHANDLER_H
// #define RFIDHANDLER_H

// #include <MFRC522.h>

// void setupRFID(MFRC522 &rfid);
// bool readRFIDCard(MFRC522 &rfid, String &cardID);

// #endif

