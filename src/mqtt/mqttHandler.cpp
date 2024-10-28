// #include "mqttHandler.h"
// #include <Arduino.h>

// void setupMQTT(PubSubClient &client)
// {
//     client.setCallback(handleMQTTMessage); // Set callback for message processing
// }

// void mqttReconnect(PubSubClient &client)
// {
//     while (!client.connected())
//     {
//         if (client.connect("ESP32_DoorClient"))
//         {
//             client.subscribe("action");
//         }
//         else
//         {
//             delay(5000);
//         }
//     }
// }

// void handleMQTTMessage(char *topic, byte *message, unsigned int length)
// {
//     String msg;

//     for (unsigned int i = 0; i < length; i++)
//     {
//         msg += (char)message[i];
//     }
//     if (String(topic) == "action")
//     {
//         if (msg == "open")
//         {
//             //LOGIC ESP32 open door
//             //once done, publish opened door
//             publishDoorStatus(client, "opened");
//         }
//         else if (msg == "close")
//         {
//             //LOGIC ESP32 open door
//             //once done, publish opened door
//              publishDoorStatus(client, "closed");
//         }
//     }
// }


// void publishDoorStatus(PubSubClient &client, const char *status)
// {
//     client.publish("doorStatus", status);
// }

