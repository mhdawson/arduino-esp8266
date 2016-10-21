#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// device specifics
#include "WirelessConfig.h"

#define BLINK_INTERVAL_SECONDS 1
#define LED_PIN D6
#define MESSAGE_WAITING_TOPIC "phone/messageWaiting"

IPAddress server(mqttServer[0], mqttServer[1],
                 mqttServer[2], mqttServer[3]);

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;
PubSubClient client(wclient, server);

boolean messageWaiting;
boolean ledOn;

void callback(const MQTT::Publish& pub) {
  if(0 == strcmp(pub.payload_string().c_str(), "1")) {
    messageWaiting = true;
  } else {
    messageWaiting = false;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  messageWaiting = false;
  ledOn = false;
  
  Serial.begin(115200);
  Serial.println("starting");
  
  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);

  client.set_callback(callback);
}


void loop() {
  client.loop();

  // make sure we are good for wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
  
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      return;
    }
    Serial.println("WiFi connected");
  }

  if (!client.connected()) {
    if (client.connect("sensclient")) {
      client.subscribe(MESSAGE_WAITING_TOPIC);
    }
  }

  if ((true == messageWaiting) && (false == ledOn)) {
    digitalWrite(LED_PIN, HIGH);
    ledOn = true;
  } else {
    digitalWrite(LED_PIN, LOW);
    ledOn = false;
  }
                 
  delay(BLINK_INTERVAL_SECONDS * 1000);

}
