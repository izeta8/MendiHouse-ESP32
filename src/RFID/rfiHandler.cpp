// rfidHandler.cpp
#include "rfidHandler.h"
#include <Arduino.h>

void setupRFID(MFRC522 &rfid)
{
    rfid.PCD_Init();
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }
}

bool readRFIDCard(MFRC522 &rfid, String &cardID)
{
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    {
        return false;
    }
    for (byte i = 0; i < 4; i++)
    {
        cardID += String(rfid.uid.uidByte[i], HEX);
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return true;
}
