#include <WiFi.h>
#include <esp_wifi.h>
#include "../arduino_secrets.h"

const char* ssid = SECRET_SSID;

const char* password = SECRET_PSW;

void setup(){

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();
 
  Serial.print("\nDefault ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.println("Connecting to WiFi..");

  }

  Serial.println("Connected to the WiFi network");
}
 
void loop(){

}