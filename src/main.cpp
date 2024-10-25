#include <Arduino.h>  
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Definir los pines SPI
#define SS_PIN 21    // GPIO 21
#define RST_PIN 22   // GPIO 22
#define MOSI_PIN 23  // GPIO 23
#define MISO_PIN 19  // GPIO 19
#define SCK_PIN 18   // GPIO 18

const char* ssid = "AEG-IKASLE";       // Wi-Fi SSID
const char* password = "Ea25dneAEG";  // Wi-Fi Password
const int mqtt_port = 8883;            // Port for MQTT over TLS/SSL


WiFiClientSecure espClient;  // Secure Wi-Fi Client

MFRC522 rfid(SS_PIN, RST_PIN); // Instancia de la clase

MFRC522::MIFARE_Key key;


// Array para almacenar el NUID
byte nuidPICC[4];

/**
 * Rutina auxiliar para mostrar un array de bytes en formato hexadecimal.
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) {
      Serial.print(" 0");
    } else {
      Serial.print(" ");
    }
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Rutina auxiliar para mostrar un array de bytes en formato decimal.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}

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

void setup_rfid(){
  Serial.begin(115200);
  delay(1000); // Esperar para asegurar que el Monitor Serial esté listo
  Serial.println(F("Iniciando el lector MFRC522..."));

  // Inicializar el bus SPI con pines específicos
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN); 
  Serial.println(F("SPI iniciado."));

  // Inicializar el lector MFRC522
  rfid.PCD_Init(); 
  Serial.println(F("MFRC522 inicializado."));

  // Inicializar la llave por defecto (todos 0xFF)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Escaneando NUID de MIFARE Classic..."));
  Serial.print(F("Usando la llave:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
}

void setup() {
    Serial.begin(115200);  // Iniciar comunicación serie
    setup_wifi();          // Conectar a Wi-Fi
    setup_rfid();
}

void loop() {
     //Serial.println(F("Esperando una nueva tarjeta..."));

  // Reiniciar el loop si no hay una nueva tarjeta presente
  if (!rfid.PICC_IsNewCardPresent()) {
    delay(500); // Esperar medio segundo antes de volver a intentar
    return;
  }

  Serial.println(F("Tarjeta detectada. Leyendo..."));

  // Verificar si el NUID ha sido leído
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println(F("Error al leer la tarjeta."));
    return;
  }

  Serial.print(F("Tipo de PICC: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Verificar si el PICC es de tipo MIFARE Classic
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Tu tarjeta no es de tipo MIFARE Classic."));
    return;
  }

  // Verificar si es una nueva tarjeta comparando el UID
  bool nuevaTarjeta = false;
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != nuidPICC[i]) {
      nuevaTarjeta = true;
      break;
    }
  }

  if (nuevaTarjeta) {
    Serial.println(F("Se ha detectado una nueva tarjeta."));

    // Almacenar el NUID en el array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("El NUID de la tarjeta es:"));
    Serial.print(F("En hexadecimal: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("En decimal: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else {
    Serial.println(F("Tarjeta ya leída anteriormente."));
  }

  // Detener la comunicación con la tarjeta
  rfid.PICC_HaltA();

  // Detener la encriptación en el lector
  rfid.PCD_StopCrypto1();

  delay(1000); // Esperar un segundo antes de la siguiente lectura
}



