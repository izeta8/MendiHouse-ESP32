#include <Arduino.h>  

#ifndef RFID_H
#define RFID_H

void readRFID();
void rfidBegin();
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);

#endif
