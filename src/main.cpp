#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h> //system to read files and use portion of flash memory of ESP32 for it

// Definir los pines SPI
#define SS_PIN 21   // GPIO 21
#define RST_PIN 22  // GPIO 22
#define MOSI_PIN 23 // GPIO 23
#define MISO_PIN 19 // GPIO 19
#define SCK_PIN 18  // GPIO 18

const char *ssid = "AEG-IKASLE";          // Wi-Fi SSID
const char *password = "Ea25dneAEG";      // Wi-Fi Password
const int mqtt_port = 1883;               // Port for MQTT over TLS/SSL
const char *mqtt_server = "10.80.128.11"; // local mosquitto runs in ip machine network

WiFiClient espClient;     // Secure Wi-Fi Client
PubSubClient client(espClient); // MQTT client

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

MFRC522 rfid(SS_PIN, RST_PIN); // Instancia de la clase
MFRC522::MIFARE_Key key;

// Array para almacenar el NUID
byte nuidPICC[4];

/**
 * Rutina auxiliar para mostrar un array de bytes en formato hexadecimal.
 */
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

/**
 * Rutina auxiliar para mostrar un array de bytes en formato decimal.
 */
void printDec(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32_MendiHouse"))
    {
      Serial.println("connected");
      // Once connected, you can subscribe or publish
      client.subscribe("esp32/test");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); // Iniciar conexión Wi-Fi

  int attempts = 0; // Contador de intentos
  while (WiFi.status() != WL_CONNECTED && attempts < 30)
  { // Hasta 30 intentos (30 segundos)
    delay(1000);
    Serial.print("Attempt ");
    Serial.println(attempts);
    Serial.println(WiFi.status()); // Imprime el estado de la conexión
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); // Imprime la IP asignada
  }
  else
  {
    Serial.println("Failed to connect to WiFi");
  }
}

void setup_rfid()
{
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
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Escaneando NUID de MIFARE Classic..."));
  Serial.print(F("Usando la llave:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
}

void setup_ssl()
{
  // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  // Load CA, Device Certificate, and Private Key from SPIFFS
  String caCert = readFile("/certificates/ca.crt");
  String deviceCert = readFile("/certificates/device.crt");
  String deviceKey = readFile("/certificates/device.key");

  // Assign the certificates to the client
  // espClient.setCACert(caCert.c_str());
  // espClient.setCertificate(deviceCert.c_str());
  // espClient.setPrivateKey(deviceKey.c_str());
}

void setup()
{
  Serial.begin(115200); // Iniciar comunicación serie
  setup_wifi();         // Conectar a Wi-Fi
  setup_rfid();
  // setup_ssl();           // Setup SSL Certificates

  client.setServer(mqtt_server, mqtt_port); // Set the MQTT broker and port
}

void loop()
{

  if (!client.connected())
  {
    reconnect(); // Try to reconnect if disconnected
    Serial.println("Reconnecting");
  }
  client.loop(); // Ensure the client maintains its connection
                 
  // Serial.println(F("Esperando una nueva tarjeta..."));

  // Reiniciar el loop si no hay una nueva tarjeta presente
  if (!rfid.PICC_IsNewCardPresent())
  {
    delay(500); // Esperar medio segundo antes de volver a intentar
    return;
  }

  Serial.println(F("Tarjeta detectada. Leyendo..."));

  // Verificar si el NUID ha sido leído
  if (!rfid.PICC_ReadCardSerial())
  {
    Serial.println(F("Error al leer la tarjeta."));
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

  // Verificar si es una nueva tarjeta comparando el UID
  bool nuevaTarjeta = false;
  for (byte i = 0; i < 4; i++)
  {
    if (rfid.uid.uidByte[i] != nuidPICC[i])
    {
      nuevaTarjeta = true;
      break;
    }
  }

  if (nuevaTarjeta)
  {
    Serial.println(F("Se ha detectado una nueva tarjeta."));

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
  }
  else
  {
    Serial.println(F("Tarjeta ya leída anteriormente."));
  }

  // Detener la comunicación con la tarjeta
  rfid.PICC_HaltA();

  // Detener la encriptación en el lector
  rfid.PCD_StopCrypto1();

  delay(1000); // Esperar un segundo antes de la siguiente lectura
}

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
// "MIIENTCCAh0CFDdhMVl0XlHJot8yKR3G+iWeWdX3MA0GCSqGSIb3DQEBCwUAMFkx\n"
// "CzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRl\n"
// "cm5ldCBXaWRnaXRzIFB0eSBMdGQxEjAQBgNVBAMMCWthb3Rpa2FDQTAeFw0yNDEw\n"
// "MjMwNzE4NTFaFw0yNTEwMjMwNzE4NTFaMFUxCzAJBgNVBAYTAkFVMRMwEQYDVQQI\n"
// "DApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQx\n"
// "DjAMBgNVBAMMBUVTUDMyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\n"
// "rPPFWqDcP7mPAINE34o2HlMHcYRNp/oGkVXBBxwdrD8brDaZD/zD2buNXaL1ZddS\n"
// "xkNlGAAwz8lcsIv2QU735EgBh4Hjj1vBzSvBBi56iz09kyrBwD9xRhQ3Ax8lHAmE\n"
// "9GXzqDIiIaETVt4XPhCcU+DjjhlWV57vAO4q8NkmDrpdsTO2mWIIiIoJoPj2J+S6\n"
// "ul3AHqyaPc7nhqhCCKJUbXsZTDVROWnVgW07BGwwV7MBZ/Rp4+WMDe0eWAW6BQyK\n"
// "rJzA++pZ7ivIJt1kPCT9S0yi+7bC+XgzEpfAClnczeGg0mmqRBDD+OFMXgR5sfmr\n"
// "KbtFHIfRobzUa/eJ8A8e5QIDAQABMA0GCSqGSIb3DQEBCwUAA4ICAQAojPgpDnuN\n"
// "oILb25nJRmd0Vw/UrzKQwxpe5wK7TwcfQF3SpANq9prEJIpcfNfvJjmWfCiyDPp5\n"
// "LH89duxJKdHeBDQAFiyYOiMTms19AqM2ArzxOBorOIxZSKa03ask35JwdkX95Z7i\n"
// "OVWJ5KJoMGZtbHhOqHt6U8qJU/wyf9iJMoK3NkBzvYxf6bNEvHS52ta0C+p5y1l9\n"
// "5zZT1zYeKIZ0YVGzpnlyvZ87JHn20qH7o7+WBXE9oyIrF755DvAvlCHkgAfTsHeu\n"
// "lAVIrkME7QG+aUUpVyJPjMWk6DOQgGbvD7h2Ao/s9lqzU8kp6nlA9Jx+pUtDDqgP\n"
// "Oc0OVhzoHR4M0q7vGDvmpedRG33vNxfln4WRDbt0jBbx4IZ7V3CAWkEkZkwgZB2u\n"
// "rw56OIjEOu+Z/Lw8fDSM05v3Jn4fLXcZ/oqqDMinz4fkcDq8Z1F0PsgJ3pd4ThXv\n"
// "+/lizWZUg0eDXbW2AfGUctB/9/bH5ZSa+BgowIXRTWbdm7AKtILp+kdItwvXY8Zo\n"
// "hruFbizb4nBzaOLq3j3vFO9nEGFXXET4IkbkEFq6cFVLSg5HNAKzJRybSOXYr1jM\n"
// "wLNsHp5zLwUxaEmJpHJPntQXU3qb4GxnmElc3ItAHAmB5IfBp/h7TQxF82BWIa4W\n"
// "hw0LJLkBm3InB6A32AwluvkMv7VNEJW1RA==\n"
// "-----END CERTIFICATE-----\n";

// const char* ca_crt =
// "-----BEGIN CERTIFICATE-----\n"
// "MIIFkzCCA3ugAwIBAgIUFCJbSK7PfavnPaUB/VzUDKbf2FEwDQYJKoZIhvcNAQEL\n"
// "BQAwWTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n"
// "GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDESMBAGA1UEAwwJa2FvdGlrYUNBMB4X\n"
// "DTI0MTAyMzA2NTg0MFoXDTM0MTAyMTA2NTg0MFowWTELMAkGA1UEBhMCQVUxEzAR\n"
// "BgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5\n"
// "IEx0ZDESMBAGA1UEAwwJa2FvdGlrYUNBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8A\n"
// "MIICCgKCAgEAmXoikRl3ifIhmFNff3L/UI/hcXVt4bGokaReUa3azltIcKer4BP2\n"
// "2QS4d7qslHlDaqDkXJXAIlaVfr/E0MnodZtFDzTNaKnoEakJbgUshTORLxvfxeW7\n"
// "fp+LvE5HL40jUyO8SihKkFCxKjGFZf0ahatAF6Tk/te5gBxW5TpX2MA7q3suymRB\n"
// "ZaurbvRDgQVl+ZErOQZ5zBaYIlpSdsUdtULSe27kEG1lF1Ws+w+E4TPFfcoGZ6Zs\n"
// "bQUL8hre+h2KE9/gSmC+6x00vAhj6+UHRELAsp4UdkxrUJQ570EFWVFJfedAgV4L\n"
// "L4A1CbOxeV4+M5A4gMErALyWzyA+SR2GH6U3HX4z48kPSFI42LGwtQhvs11VW31y\n"
// "tpgV9ylqOK2INYB8YqcmReY/Q6eUu1OtH7vMs1CFCgw5ZkZW8E0tyYuoocg5XerI\n"
// "N1ZGAq09i7JesVmZOpVAfdWPoUmH5uIdEfWDzgCkEoO+HMn15QWhibcdZz8XdQvt\n"
// "+bh5A8V7huKnyqeMO2Z97rCkG90lic/lu9PpHSD23dBsRk0H3J9OIyLzG+VJ128K\n"
// "0KpUa/u76OsMxe/dbXBesU3IC+20MhX3V75fjskeCdsMWY/oG/vzjIJ+DKL+dk6E\n"
// "wFmQZgrEKRlf1yf7Ezb75mY7kQnvsmwkZxd9fZhm02kov9Pm2oM2IesCAwEAAaNT\n"
// "MFEwHQYDVR0OBBYEFL1bxIppTpNwyubX8CQ8trN2NHQAMB8GA1UdIwQYMBaAFL1b\n"
// "xIppTpNwyubX8CQ8trN2NHQAMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n"
// "BQADggIBAF0bTXedB3r5h/y4YgcYb0n6kp0gCCMcKjyro2MfZx7jzyC3FCyTjcUT\n"
// "OzQka/SnP9j+8sHQJ6xubWzl8BarSGibeVjCbT0KoTM9KmnIEKFG2DS5RmnyXLSQ\n"
// "A6dTPA21JZutu2gnT7+sii/8pMngCwRIoUZDpNIOBACi93lw0ntR5D9kwocQM4pv\n"
// "FC8LOcXU8rUMkd17A10w5R9G0X6D3OCRmZT3P0UmdLY+D3CfSmPTKXH5I/VAEtdT\n"
// "vnzlAXLJLMxZDPGyOawzBOn8K0tudHZvjp/d/9zTeIqhjaeRK8wCFLE+bynk5JRP\n"
// "aB4OX+WzGpKWIXBHP9yDjfPLQLr0MErVuShlpfLxqeNLU6B4j0nOTGKG1zFdz2uA\n"
// "RbdoQ8zNe4/pJItBBua2YP4dBimjGMwYI5nadFf2t02nKx6328PUFnPB43TlXvOX\n"
// "69uWvP3K6Uwf85gUA4hIwuzD7BJcl3xPgd7gn9U/YH0umxAjxxYshMYZ44mc755c\n"
// "Um3hCC9z7c+ugXDC6Bn0BM8IBGq4GEfMV6zEz7esfrR1KN50nXh9KXRr9RbZj6cA\n"
// "mJuNefa1SCQqXa09lxqiTv7W9BjSvsYF/MEynwlCd6oUc/0NF8PqHYflWBk2Zrpa\n"
// "+5x2z42LNTTGPJYhECtsm5U0gFm3zubA+n/JFb3h8CMSlPj8K6MB\n"
// "-----END CERTIFICATE-----\n";