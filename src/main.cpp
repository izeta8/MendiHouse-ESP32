#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <SPIFFS.h>
#include "esp_wifi.h"  // Include ESP-IDF WiFi functions

#define SS_PIN 21   // GPIO 21
#define RST_PIN 22  // GPIO 22
#define MOSI_PIN 23 // GPIO 23
#define MISO_PIN 19 // GPIO 19
#define SCK_PIN 18  // GPIO 18

#define SERVO_PIN 13

Servo myServo;

const char *ssid = "AEG-IKASLE";          
const char *password = "Ea25dneAEG";      
const int mqtt_port = 1883; //8883 with SSL               
const char *mqtt_server = "10.80.128.2"; 

///////////////////////////////////////////////////////////////////////
// const char* device_key =
// "-----BEGIN PRIVATE KEY-----\n"
// "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCs88VaoNw/uY8A\n"
// "g0TfijYeUwdxhE2n+gaRVcEHHB2sPxusNpkP/MPZu41dovVl11LGQ2UYADDPyVyw\n"
// "i/ZBTvfkSAGHgeOPW8HNK8EGLnqLPT2TKsHAP3FGFDcDHyUcCYT0ZfOoMiIhoRNW\n"
// "3hc+EJxT4OOOGVZXnu8A7irw2SYOul2xM7aZYgiIigmg+PYn5Lq6XcAerJo9zueG\n"
// "qEIIolRtexlMNVE5adWBbTsEbDBXswFn9Gnj5YwN7R5YBboFDIqsnMD76lnuK8gm\n"
// "3WQ8JP1LTKL7tsL5eDMSl8AKWdzN4aDSaapEEMP44UxeBHmx+aspu0Uch9GhvNRr\n"
// "94nwDx7lAgMBAAECggEAAIyFg9Qe3tTn70h7X3pA0bfRvjp9RJs2ih+LjUo5jeJl\n"
// "ZBaIRdS3imld+X4a6UfVq2trVrgpb39kG+tVzL9DaiYQWy2+WyYfZ1dMNYjQOsDn\n"
// "ASCRnNqWsXI++nOO2pftJBluvInsMc8hEJWD3V6QIHQGRN0vuPd4KriWykzmIgXU\n"
// "DgGHduGoY0rNh9S0httZ8zAtKA5aAVL7K/2eCXZYF2ryTRT+dNAV3CXM0qmd0twP\n"
// "XT1O9hwyQLCpBxdRResULzK6XdsqS7gnUT5vNe4va0wIp8bYWJtYy2oa7oyJn7Ti\n"
// "qk8p1ZaHVyjMbYBERiDYeJiw86Zz7DhhHzO3iohaAQKBgQC7GZvKuky+iB+BfZC4\n"
// "4DWNUiFURwk/s7f6t64mBPiMwdFUFMUNXe+48MfbJJ2B2I8TZhHHX8rZN/5BNPN5\n"
// "a6cI5oofxQOylJ7audm9yYNFH64RdSVPWKSY9PBNk5nIGmyFfCjQUY1prvyL8ZyU\n"
// "+8sGiZBKRgnH6L5MGlLyX+WVrQKBgQDspGnN09schlXTqoBqIHMbgyhOBI0f3JFk\n"
// "OlA6r4B7p+yT82IQZx405MG365MBmlxFBfsAxSQZIR36+0MI7KfXli4+CLU8ajpr\n"
// "tLRSTnPAMTc+EQd0TYMbo+5n0ATKjehXZ7Hnyq6EhMyk+lIEObo2iwuJL9ct0Tzh\n"
// "cxJyVTelGQKBgEsAHfprKd5qtK50xjHF9tGara+Z9jQp98MyJEYrUfMisqXQ2Zjd\n"
// "If5xzrui9tyZEemFzfStCqR+lJZRApSWudOQjqfjkvLgLrmhLI6um8d2xpFK81TW\n"
// "RMznyvheBZQ4O25a8gVn25cUxTnX9ZunP6B0Pnn3j24qdzYMi1mLZX3VAoGBAL8n\n"
// "NFxSVP1eujen0+FmYvUHZmhXHtRs7HkE7cfp+qRoih8dC8rqMSlS2/TAc1hpIz2S\n"
// "TiZaGQW/jaMukgDEgfQQ2Tu89wFdYeGc78hInRHT76bQRLA66LxMGIl9LFGefd8U\n"
// "4U22mHBtgY2Gsve81eG3QdNIdM4K5d3LgUwf8DoJAoGAKDpkajB8FrU73ORQJp2N\n"
// "L4V+wInOxEeUaUbeXbyvJIOFBmgKkYeJD4Q4dxVmjipo96VUrhbDBIuZETgEWYN/\n"
// "7KVvtU/vIdgg8fUzS3hOX5ssnNhae2ebzDRj9SlnnKvXSqRaY/X2qcwMpP2iCY8p\n"
// "6R55mOZEF9WthcMMIQr/gj4=\n"
// "-----END PRIVATE KEY-----\n";

// const char* device_crt =
// "-----BEGIN CERTIFICATE-----\n"
// "MIIEZjCCAk4CFDFJOJOeCY/3NWwPFe28nvzIOCBAMA0GCSqGSIb3DQEBCwUAMIGJ\n"
// "MQswCQYDVQQGEwJFVTERMA8GA1UECAwIR0lQVVpLT0ExETAPBgNVBAcMCERPTk9T\n"
// "VElBMQwwCgYDVQQKDANBRUcxEDAOBgNVBAsMB0tBT1RJS0ExEDAOBgNVBAMMB0tB\n"
// "T1RJS0ExIjAgBgkqhkiG9w0BCQEWE29za2FyLmNhbHZvQGFlZy5ldXMwHhcNMjQx\n"
// "MTA1MTE0NzIxWhcNMjUxMTA1MTE0NzIxWjBVMQswCQYDVQQGEwJBVTETMBEGA1UE\n"
// "CAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRk\n"
// "MQ4wDAYDVQQDDAVFU1AzMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
// "AKzzxVqg3D+5jwCDRN+KNh5TB3GETaf6BpFVwQccHaw/G6w2mQ/8w9m7jV2i9WXX\n"
// "UsZDZRgAMM/JXLCL9kFO9+RIAYeB449bwc0rwQYueos9PZMqwcA/cUYUNwMfJRwJ\n"
// "hPRl86gyIiGhE1beFz4QnFPg444ZVlee7wDuKvDZJg66XbEztpliCIiKCaD49ifk\n"
// "urpdwB6smj3O54aoQgiiVG17GUw1UTlp1YFtOwRsMFezAWf0aePljA3tHlgFugUM\n"
// "iqycwPvqWe4ryCbdZDwk/UtMovu2wvl4MxKXwApZ3M3hoNJpqkQQw/jhTF4EebH5\n"
// "qym7RRyH0aG81Gv3ifAPHuUCAwEAATANBgkqhkiG9w0BAQsFAAOCAgEAzsnDwPTh\n"
// "ZTYVyOm4g4yld0OL2IakcfnE0baV8LjGynnfJVT8jHRvU6n+GzmO09TsvkyJWxkR\n"
// "iVl+G/TJHvg5ZARyyTyCaMcXbpLt2uLV9cQwDv98Ji+faM123bHnBWK8F2zyYd8X\n"
// "dcL1UFlPrCQ96Kcp6H7/+FAda62AKfAzhoLXSPe3puW3M5bM9ey4wldSaVkgCiiQ\n"
// "5izC9BvA8FCUVTk8ePW/6adHwXLq36xnkav3CXUsnzwAABlFkBCecpw6eh8Wk1bQ\n"
// "i7UIJ3e22YqBVxiWfiY9BLd1PjsKObW4JFrM5hhO6QRKnTXo/tpsHg7yQp2rlG8n\n"
// "q+iXLCc+sadPO0b2+0WudtRnsmOWIFnDaKULsafzufqHEc5MGBUR/QQfMIf++Pm7\n"
// "WOiaLuRwBX5V69xMSJpUGVHU5QIaZuT6XVfc4SyUD+VV41T/ucZpvcb8zyM0WlA2\n"
// "mEJLf06yO5dOyvmsulEyQEdX2pXefwKZRtRUKnC+Mn1xAmg+4BvtHWEfZetEVSgm\n"
// "unhmaAxYz7UnwZfPDsAd1hdTQr86R19ojCZEmfq+H1C7wW2osIwdKW9FRysc3g+L\n"
// "hXz91e/ZADkgnCAXbej02O6hOaUhy5D0a4gBTqQPb9hNpu7bS+i4Z+JgyQ1ECtwb\n"
// "hje0vz27mOQAh20BR5xe4CZPNn+UVCs4HSs=\n"
// "-----END CERTIFICATE-----\n";

// const char* ca_crt =
// "-----BEGIN CERTIFICATE-----\n"
// "MIIF9TCCA92gAwIBAgIUTn+K4jmgqp3YxzM+grGiA87ZO1YwDQYJKoZIhvcNAQEL\n"
// "BQAwgYkxCzAJBgNVBAYTAkVVMREwDwYDVQQIDAhHSVBVWktPQTERMA8GA1UEBwwI\n"
// "RE9OT1NUSUExDDAKBgNVBAoMA0FFRzEQMA4GA1UECwwHS0FPVElLQTEQMA4GA1UE\n"
// "AwwHS0FPVElLQTEiMCAGCSqGSIb3DQEJARYTb3NrYXIuY2Fsdm9AYWVnLmV1czAe\n"
// "Fw0yNDExMDUxMTA3NDhaFw0zNDExMDMxMTA3NDhaMIGJMQswCQYDVQQGEwJFVTER\n"
// "MA8GA1UECAwIR0lQVVpLT0ExETAPBgNVBAcMCERPTk9TVElBMQwwCgYDVQQKDANB\n"
// "RUcxEDAOBgNVBAsMB0tBT1RJS0ExEDAOBgNVBAMMB0tBT1RJS0ExIjAgBgkqhkiG\n"
// "9w0BCQEWE29za2FyLmNhbHZvQGFlZy5ldXMwggIiMA0GCSqGSIb3DQEBAQUAA4IC\n"
// "DwAwggIKAoICAQDRHYLECpA1vHB4oBoqjGn2rJ1vwParc3FIGtBeDDTkKvQWT0rX\n"
// "p86a30WGIJW1/AugLqJNyNuU3aiFS/orK1CM2lPe7QP9TwbbW/Jyu5Hnff6f91ZW\n"
// "fmzUDsPTI8pCUE0GAz2bsxzA59XGSaMIllbX0cmMUfETU03QdIZOmx7v+fqkX3vY\n"
// "sRrDSZ1tuRo+t9MRfidEV17S61/kwDt9WfE56mtREXGr/ogTYnACiA3a5mcsUe84\n"
// "tYOKXsMFDV3Xh4iDotPS2pqPHnWhGbculHeFO3NVgsQkt83hHC6df98Tal1OI8cG\n"
// "BUTfno6viAa+fSRXVhxRlgVKPcSdqIH9PdU27tEemio7qPrbS3yvInSUt+XtXE0U\n"
// "UiPqHQAx8e1bosGFqaOqnGR98YtePo5AwRrl2nTJxbvyhoPg8D0DVAFjzv8UgCHN\n"
// "Obo52y1Qb8PqfucFQzltbOVihRmskQbRBg6XORg+hKstDWcZBm6PdgusK2l5lC+H\n"
// "gChnlaA22XcSjfqSv7iP5nnU15fXm1L33iZIY9qzKfBg2Zzd8Le4mvn7wGbnKY5/\n"
// "AfEyLyxVl/5JHeCnHDGjQayhh6ojZoMR+XgPYmdDqT7OcDAcLemZVPDQvHpeScSq\n"
// "ECrPN1J7tbwQsueF9T+DwvbvDPeOcfRAZxgQUico+6q+STWmCQDHaVlAtQIDAQAB\n"
// "o1MwUTAdBgNVHQ4EFgQU4olSyYbprVwO0bkiAM/9eRXSiXkwHwYDVR0jBBgwFoAU\n"
// "4olSyYbprVwO0bkiAM/9eRXSiXkwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0B\n"
// "AQsFAAOCAgEAkQMv07kbzRaG55T6Br0WbtUsDtuDc9m6JWBwaItksPcgDajTgZJc\n"
// "71x53Ie94X0JT/wW0MsprD+cUqR6MkHAJpKjSxon7+BxIxF3N/YQbs9dCVDddMqn\n"
// "EJ1PhFrmV9Mt/zhhV3p8u8BvuTdgKHtkLhHrwcmSTIl9Ed6YJ+a/dzqkbnTdiEaU\n"
// "WZZq7+20mM7xXUxc6uoCqAnyJtDKUV1Gltiff8arEiqJrSa5J6thvD2b3G5ErFID\n"
// "lXcj2pEAdH+Ml+Ggp6v0181aeGS/bYQTJXhloYOyudnoSt1X35dvT3HSCsD5T3iT\n"
// "cPtu5sX6HJxSrzt5c0Hgm2XHRxGW25XVao1dUn4GqNJ/qMrtqmT9SH4E8Nhse6d4\n"
// "H1E362x4RlmsaQbmYNh9toOXqLtBXTjw24qrnprMdSfymiJvJg+u9Itp++m+h87b\n"
// "VJPuiFmLeVhfXDHOxl/gvoCgDLfIbwfDQaefEz6WNO898jsciiZn66cCzGu4b+eC\n"
// "UGry77DBM08LgoQDBuieDHiMaJ3dgpmvyoWTaVt/oOo2rS1r+o7n9UFW3LgmUiGh\n"
// "XKvpsnMBXXMoKEljtahuYWOPqoi+B7wM9YB6wGYnoF8ML3b9XFo9EuJUoRro4JX1\n"
// "+F7Gra0qxN7kM9oc6BNlY3GQZwQpG4uRZXwBcp2doptJvSpX/o5F1zY=\n"
// "-----END CERTIFICATE-----\n";


////////////////////////////////////////////////////////////////////////

WiFiClient espClient;
// WiFiClientSecure espClient; //SSL      
PubSubClient client(espClient);  

#define BUZZER_PIN 12 
#define BUZZER_CHANNEL 1
#define BUZZER_RESOLUTION 8

#define LED_GREEN_PIN 26 
#define LED_RED_PIN 27   
#define LED_YELLOW_PIN 25  

bool actionOpen = false;
bool tokenCondition = false;

bool isPending = false;
int  doorAngle = 0;

byte nuidPICC[4] = {0, 0, 0, 0}; 

MFRC522 rfid(SS_PIN, RST_PIN); 
MFRC522::MIFARE_Key key;

// Function definitions
void setup_wifi();
void setup_rfid();
void setup_ssl();
void beep(int times, int duration, int pause, int frequency);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnect_mqtt();
void handleRFID();
void openDoor();
void closeDoor();
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);
void indicateSuccess();
void indicateError();
void stopSuccess();
void stopError();
void indicateNotInScreen();
void stopNotInScreen();
String byteArrayToHexString(byte *buffer, byte bufferSize);

void setup()
{
  Serial.begin(115200); 
  setup_wifi();  
  setup_rfid();
  setup_ssl();

  pinMode(BUZZER_PIN, OUTPUT);      
  digitalWrite(BUZZER_PIN, LOW);   

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);

  client.setServer(mqtt_server, mqtt_port); 
  client.setCallback(mqtt_callback); 
}

void loop()
{
  if (!client.connected())
  {
    reconnect_mqtt(); 
  }
  client.loop(); 

  handleRFID();
}

// -------------------- --------//
// -----   CERTIFICATES   ----- //
// -----------------------------//

void setup_ssl() {
  // espClient.setCACert(ca_crt);
  // espClient.setCertificate(device_crt);
  // espClient.setPrivateKey(device_key);
}


// -------------------- //
// -----   MQTT   ----- //
// -------------------- //

void reconnect_mqtt()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP32_MendiHouse"))
    {
      Serial.println("connected");
      client.subscribe("MendiHouse/doorAction");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

// Function to handle received MQTT messages.
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char messageBuffer[length + 1];
  memcpy(messageBuffer, payload, length);
  messageBuffer[length] = '\0'; 

  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(messageBuffer);

  if (String(topic) == "MendiHouse/doorAction") {
    StaticJsonDocument<200> doc;

    DeserializationError error = deserializeJson(doc, messageBuffer);
    if (error) {
      Serial.print(F("Error parsing JSON: "));
      Serial.println(error.f_str());
      return;
    }

    const char* action = doc["action"];
    const char* token = doc["token"];

   if (action != nullptr) {
      String status;
      if (strcmp(action, "open") == 0) {
        Serial.println("Open command received.");
        indicateSuccess();
        beep(1, 100, 100, 800);
        delay(1500);
        stopSuccess();
        openDoor();
        status = "opened";
      } else if (strcmp(action, "close") == 0) {
        Serial.println("Close command received.");
        closeDoor();
        status = "closed";
      } else if (strcmp(action, "notInScreen") == 0){
        Serial.println("The acolyte is not in the tower door.");
        indicateNotInScreen();
        beep(2, 100, 100, 800); 
        delay(1500);
        stopNotInScreen();
         isPending = false;
      }else {
        Serial.println("Action error.");
        isPending = false;
        indicateError();
        beep(2, 100, 100, 800); 
        delay(1500);
        stopError();
         isPending = false;
        return; 
      }

      Serial.print("Token associated with the command: ");
      Serial.println(token);

      // Construct JSON for doorStatus
      StaticJsonDocument<200> docStatus;
      docStatus["doorStatus"] = status;
      docStatus["token"] = token;

      String output;
      serializeJson(docStatus, output);

      // Publish the JSON to doorStatus
      bool publishSuccess = client.publish("MendiHouse/doorStatus", output.c_str());
      if (publishSuccess) {
        Serial.println(F("Door status successfully published to MQTT topic 'MendiHouse/doorStatus'."));
      } else {
        Serial.println(F("Failed to publish door status to MQTT topic 'MendiHouse/doorStatus'."));
      }
    } else {
      Serial.println("Fields 'action' or 'token' are missing in the JSON.");
    }
  }
}


// -------------------- //
// -----   WIFI   ----- //
// -------------------- //

// Function to connect to Wi-Fi.
// void setup_wifi()
// {
//   delay(10);
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);

//   WiFi.begin(ssid, password); 

//   int attempts = 0; 
//   while (WiFi.status() != WL_CONNECTED && attempts < 30)
//   { 
//     delay(1000);
//     Serial.print("Attempt ");
//     Serial.println(attempts);
//     Serial.println(WiFi.status()); 
//     attempts++;
//   }

//   if (WiFi.status() == WL_CONNECTED)
//   {
//     Serial.println("");
//     Serial.println("WiFi connected");
//     Serial.print("IP Address: ");
//     Serial.println(WiFi.localIP()); 
//   }
//   else
//   {
//     Serial.println("Failed to connect to WiFi");
//   }
// }

void setup_wifi()
{
  // Initialize WiFi in Station mode
  WiFi.mode(WIFI_MODE_STA);

  // Define the new MAC address
  uint8_t new_mac[] = { 0xE0, 0xE2, 0xE6, 0x0B, 0x68, 0x5C };

  // Set the new MAC address
  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, new_mac);
  if (result != ESP_OK) {
    Serial.println("Failed to set MAC address");
  } else {
    Serial.print("Custom MAC address set to: ");
    for(int i = 0; i < 6; i++) {
      if(new_mac[i] < 0x10) Serial.print("0");
      Serial.print(new_mac[i], HEX);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }

  // Proceed with connecting to WiFi
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); 

  int attempts = 0; 
  while (WiFi.status() != WL_CONNECTED && attempts < 30)
  { 
    delay(1000);
    Serial.print("Attempt ");
    Serial.println(attempts);
    Serial.println(WiFi.status()); 
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); 
  }
  else
  {
    Serial.println("Failed to connect to WiFi");
  }
}


// ---------------------------------- //
// -----   RFID (CARD READER)   ----- //
// ----------------------------------- //

//Function for initialising the RFID.
void setup_rfid()
{
  Serial.begin(115200);
  delay(1000); 
  Serial.println(F("Iniciando el lector MFRC522..."));

  // Inicializar el bus SPI con pines específicos
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  Serial.println(F("SPI iniciado."));

  // Inicializar el lector MFRC522
  rfid.PCD_Init();
  Serial.println(F("MFRC522 inicializado."));

  // Inicializar la llave por defecto (todos 0xFF)
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Escaneando NUID de MIFARE Classic..."));
  Serial.print(F("Usando la llave:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
}

void handleRFID(){
  // Reiniciar el loop si no hay una nueva tarjeta presente
  if (!rfid.PICC_IsNewCardPresent())
  {
    delay(500); // Esperar medio segundo antes de volver a intentar
    return;
  }
  if(!isPending)beep(1, 100, 100, 800);
  Serial.println(F("Tarjeta detectada."));
  Serial.print(F("Pending: "));
  Serial.println(isPending ? "true" : "false");
    
  if (!isPending && doorAngle == 0)
  {
    isPending = true;
    Serial.println(F("Leyendo..."));
    
    // Verificar si el NUID ha sido leído
    if (!rfid.PICC_ReadCardSerial())
    {
      beep(2, 100, 100, 800);
      Serial.println(F("Error al leer la tarjeta."));
      isPending = false;
      return;
    }

    Serial.print(F("Tipo de PICC: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.println(rfid.PICC_GetTypeName(piccType));

    // Verificar si el PICC es de tipo MIFARE Classic
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K)
    {
      Serial.println(F("Tu tarjeta no es de tipo MIFARE Classic."));
      return;
    }

    // Almacenar el NUID en el array
    for (byte i = 0; i < 4; i++)
    {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("El NUID de la tarjeta es:"));
    Serial.print(F("En hexadecimal: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("En decimal: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();

    String nuidHex = byteArrayToHexString(nuidPICC, 4); 
    Serial.print(F("NUID Hex String: "));
    Serial.println(nuidHex);
      

    client.publish("MendiHouse/cardId", nuidHex.c_str());
    Serial.println(F("NUID published to MQTT topic 'MendiHouse/cardId'."));
  
    // Detener la comunicación con la tarjeta
    rfid.PICC_HaltA();

    // Detener la encriptación en el lector
    rfid.PCD_StopCrypto1();
  }
}

// -------------------------- //
// -----   SERVO (DOOR) ----- //
// -------------------------- //

//Function for operating the door lock.
void closeDoor() {
  myServo.attach(SERVO_PIN, 554, 2400);
  Serial.println("Door closing...");
  // Giro de 90 a 0º. CERRAR PUERTA
  for (int i = 90; i >= 0; i--) {
    myServo.write(i);
     doorAngle = i;
    delay(20);
  }
  Serial.println("Door closed");
  isPending = false;
}

//Function to control the opening of the door.
void openDoor() {
  Serial.println("Door opening...");
  myServo.attach(SERVO_PIN, 554, 2400);
  // Giro de 0 a 90º. ABRIR PUERTA
  for (int i = 0; i <= 90; i++) {
    myServo.write(i);

    doorAngle = i;

    delay(20);
  }
  Serial.println("Door opened");
}

//Function to check and open the door if the conditions are met.
void checkAndOpenDoor() {
  if (actionOpen && tokenCondition) {
    openDoor();
    beep(1, 100, 100, 800);
    actionOpen = false;
    tokenCondition = false;
  } 
}

// ---------------------- //
// -----   BUZZER   ----- //
// ---------------------- //

//Function to beep with the buzzer.
void beep(int times, int duration, int pause, int frequency) {
  ledcWriteTone(BUZZER_CHANNEL, frequency);
  for(int i = 0; i < times; i++) {
     digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);                 
    digitalWrite(BUZZER_PIN, LOW);  
    delay(pause);              
  }
}

// ---------------------- //
// -----   LEDS   ----- //
// ---------------------- //

void indicateSuccess() {
  digitalWrite(LED_GREEN_PIN, HIGH);                     
   
}
void indicateError() {
  digitalWrite(LED_RED_PIN, HIGH);                  
}
void stopSuccess() {
  digitalWrite(LED_GREEN_PIN, LOW); 
}
void stopError() {
  digitalWrite(LED_RED_PIN, LOW); 
}
void indicateNotInScreen() {
  digitalWrite(LED_YELLOW_PIN, HIGH);                  
}
void stopNotInScreen() {
  digitalWrite(LED_YELLOW_PIN, LOW); 
}


// ------------------------- //
// -----   UTILITIES   ----- //
// ------------------------- //


//Function to Convert Byte Array to Hexadecimal String
String byteArrayToHexString(byte *buffer, byte bufferSize) {
  String hexString = "";
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) {
      hexString += "0"; 
    }
    hexString += String(buffer[i], HEX);
  }
  hexString.toUpperCase(); 
  return hexString;  
}

void printHex(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    if (buffer[i] < 0x10)
    {
      Serial.print(" 0");
    }
    else
    {
      Serial.print(" ");
    }
    Serial.print(buffer[i], HEX);
  }
}


void printDec(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}


// Function to read file from SPIFFS
String readFile(const char *path)
{
  File file = SPIFFS.open(path, "r");
  if (!file)
  {
    Serial.printf("Failed to open file for reading: %s\n", path);
    return String();
  }
  String content = file.readString();
  file.close();
  return content;
}


