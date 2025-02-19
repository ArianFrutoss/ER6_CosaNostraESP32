//Libraries
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <MFRC522.h>//https://github.com/miguelbalboa/rfid
#include <ESP32Servo.h>
#include <Arduino.h>
#include "../arduino_secrets.h"

Servo myServo;

//Constants
#define SS_PIN 5
#define RST_PIN 0
#define GPI0_PIN 16
#define CLOSE_DOOR 0
#define OPEN_DOOR 180

#define GREEN_LED_PIN 26
#define RED_LED_PIN 25 
#define BUZZER_PIN 12 

int servoAngle = CLOSE_DOOR;

const char* ssid = SECRET_SSID;
const char* password = SECRET_PSW;
String cardID = "";
const char* mqtt_server = SECRET_SERVER;
const int mqtt_port = SECRET_PORT;

const char* ca_crt = CA_CRT;
const char* esp32_crt = ESP32_CRT;
const char* esp32_key = ESP32_KEY;

bool executing = false;

//Parameters
const int ipaddress[4] = {103, 97, 67, 25};

//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

WiFiClientSecure espClient;

PubSubClient client(espClient);

void reconnect(){

  while(!client.connected()){

    Serial.print("Attempting MQTT connection...");
    
    if(client.connect("ESP32CosaNostra")){
      
      executing = false;
      client.subscribe("cosanostra/esp32/#");
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

void setup_ssl(){

  espClient.setCACert(ca_crt);
  espClient.setCertificate(esp32_crt);
  espClient.setPrivateKey(esp32_key);
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

  String cardId = "";

  for (byte i = 0; i < rfid.uid.size; i++) {

    cardId += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    cardId += String(rfid.uid.uidByte[i], HEX);
  }

  char message[100];
  cardId.toCharArray(message, cardId.length() + 1);

  cardID = cardId;
  Serial.print("Saved into CardID: ");
   Serial.println(message);
  client.publish("cosanostra/server/cardid", message);

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  executing = true;
}

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {

    Serial.print((char)payload[i]);
  }
  if(String(topic) == "cosanostra/esp32/opendoor") openDoor();
  if(String(topic) == "cosanostra/esp32/closedoor") closeDoor();
  if(String(topic) == "cosanostra/esp32/accessdenied") accessDenied();
  
}

void openDoor() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  tone(BUZZER_PIN,50);
  delay(250);  
  digitalWrite(GREEN_LED_PIN, LOW);
  noTone(BUZZER_PIN);
  Serial.println("Opening Door");
  graduallyApplyServoAngle(OPEN_DOOR);
  char message[100];
  cardID.toCharArray(message, cardID.length() + 1);
  client.publish("cosanostra/server/dooropened", message);
}

void closeDoor() {
  Serial.println("Closing Door");
  delay(1000);
  graduallyApplyServoAngle(CLOSE_DOOR);
  cardID = "";
  executing = false;
}

void accessDenied() {
  Serial.println("Access Denied");
  digitalWrite(RED_LED_PIN, HIGH);
  delay(500);
  digitalWrite(RED_LED_PIN, LOW);
  tone(BUZZER_PIN,2000);
  delay(250);
  noTone(BUZZER_PIN);
  tone(BUZZER_PIN,1000);
  delay(250);
  noTone(BUZZER_PIN);
  executing = false;
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

void graduallyApplyServoAngle(int angle)
{
  int limitedAngle = limitServoAngle(angle);
 
  while (servoAngle != limitedAngle) {
    if (servoAngle > limitedAngle) {
      servoAngle--;
    }
 
    if (servoAngle < limitedAngle) {
      servoAngle++;
    }
 
    myServo.write(servoAngle);
    delay(20);
  }
}

int limitServoAngle(int angle)
{
  return constrain(angle, CLOSE_DOOR, OPEN_DOOR);
}

void setup(){

  Serial.begin(115200);

  setup_wifi();
  setup_ssl();

  client.setServer(mqtt_server, mqtt_port);

  Serial.println(F("Initialize System"));

  //init rfid D8,D5,D6,D7
  SPI.begin();
  rfid.PCD_Init();

  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();

  myServo.attach(GPI0_PIN);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  client.setCallback(callback);
}


void loop(){
  
  if(!client.connected()){

    reconnect();
  }

  client.loop();

  if (!executing){

    readRFID();
  }
}