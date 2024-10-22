#include <WiFi.h>
#include <esp_wifi.h>

void setup(){

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();
 
  Serial.print("\nDefault ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){

}