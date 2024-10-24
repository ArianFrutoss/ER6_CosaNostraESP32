#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "../arduino_secrets.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PSW;

const char* mqtt_server = SECRET_SERVER;
const int mqtt_port = SECRET_PORT;

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

void setup(){

  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
}
 
void loop(){
  
  if(!client.connected()){

    reconnect();
  }

  client.loop();
}