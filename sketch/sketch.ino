//Libraries
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SPI.h>//https://www.arduino.cc/en/reference/SPI
#include <MFRC522.h>//https://github.com/miguelbalboa/rfid

#include "../arduino_secrets.h"

//Constants
#define SS_PIN 5
#define RST_PIN 0

const char* ssid = SECRET_SSID;
const char* password = SECRET_PSW;

const char* mqtt_server = SECRET_SERVER;
const int mqtt_port = SECRET_PORT;

//Parameters
const int ipaddress[4] = {103, 97, 67, 25};

//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

WiFiClient espClient;

PubSubClient client(espClient);

void reconnect(){

  while(!client.connected()){

    Serial.print("Attempting MQTT connection...");
    
    if(client.connect("ESP32CosaNostra")){

      Serial.println("Connected");
    }

    else{

      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");

      delay(5000);
    }
  }
}

void setup_wifi(){
  
  delay(10);

  WiFi.begin(ssid, password);

  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
}

void readRFID(void ) { /* function readRFID */

  ////Read RFID card
  for (byte i = 0; i < 6; i++) {

    key.keyByte[i] = 0xFF;
  }

  // Look for new 1 cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (  !rfid.PICC_ReadCardSerial())
    return;

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {

    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  Serial.print(F("RFID In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void printHex(byte *buffer, byte bufferSize) {

  for (byte i = 0; i < bufferSize; i++) {
    
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {

  for (byte i = 0; i < bufferSize; i++) {

    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

void setup(){

  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);

  Serial.println(F("Initialize System"));

  //init rfid D8,D5,D6,D7
  SPI.begin();
  rfid.PCD_Init();

  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
}
 
void loop(){
  
  if(!client.connected()){

    reconnect();
  }

  client.loop();
  readRFID();
}