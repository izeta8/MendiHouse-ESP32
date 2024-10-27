#include "mqttHandler.h"
#include <Arduino.h>

void setupMQTT(PubSubClient &client)
{
    client.setCallback(handleMQTTMessage); // Set callback for message processing
}

void mqttReconnect(PubSubClient &client)
{
    while (!client.connected())
    {
        if (client.connect("ESP32_DoorClient"))
        {
            client.subscribe("action");
        }
        else
        {
            delay(5000);
        }
    }
}

void handleMQTTMessage(char *topic, byte *message, unsigned int length)
{
    String msg;

    for (unsigned int i = 0; i < length; i++)
    {
        msg += (char)message[i];
    }
    if (String(topic) == "action")
    {
        if (msg == "open")
        {
            //ESP32 opens door and once done it , publish door opened
        }
        else if (msg == "close")
        {
            //ESP32 closes door and once done i , publish door closed
        }
    }
}


