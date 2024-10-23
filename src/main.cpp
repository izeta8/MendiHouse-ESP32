#include <Arduino.h>  
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <RFID/RFID.h>
#include <WiFiClientSecure.h>

const char* ssid = "Mi 10T";       // Wi-Fi SSID
const char* password = "baconmanu";  // Wi-Fi Password
const int mqtt_port = 8883;            // Port for MQTT over TLS/SSL


WiFiClientSecure espClient;  // Secure Wi-Fi Client



void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password); // Iniciar conexión Wi-Fi

    int attempts = 0;  // Contador de intentos
    while (WiFi.status() != WL_CONNECTED && attempts < 30) { // Hasta 30 intentos (30 segundos)
        delay(1000);
        Serial.print("Attempt ");
        Serial.println(attempts);
        Serial.println(WiFi.status());  // Imprime el estado de la conexión
        attempts++;
    }

    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());  // Imprime la IP asignada
    } else {
        Serial.println("Failed to connect to WiFi");
    }
}

void setup() {
    Serial.begin(115200);  // Iniciar comunicación serie
    setup_wifi();          // Conectar a Wi-Fi
}

void loop() {
    // Puedes poner más lógica aquí
}



