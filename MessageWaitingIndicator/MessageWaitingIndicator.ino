// Copyright 2016-2017 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// device specifics
#include "WirelessConfig.h"

#define BLINK_INTERVAL_SECONDS 1
#define LED_PIN D6
#define MESSAGE_WAITING_TOPIC "phone/messageWaiting"

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;
boolean messageWaiting;
boolean ledOn;

int livenessIndicator = 0;
void callback(char* topic, uint8_t* message, unsigned int length) {
  std::string messageBuffer((const char*) message, length);

  Serial.print("publish: ");
  Serial.print(livenessIndicator);
  Serial.print("-");
  Serial.println(messageBuffer.c_str());
  livenessIndicator = (livenessIndicator + 1);
  if (0 == strcmp(messageBuffer.c_str(), "1")) {
    messageWaiting = true;
  } else {
    messageWaiting = false;
  }
}

PubSubClient client(mqttServerString, mqttServerPort, callback, wclient);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  messageWaiting = false;
  ledOn = false;
  
  Serial.begin(115200);
  Serial.println("starting");
  
  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);

  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Serial.println("WiFi connected");
  }
}

int count = 0;

void loop() {
  client.loop();

  // make sure we are good for wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.reconnect();
  
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Failed to reconnect WIFI");
      Serial.println(WiFi.waitForConnectResult());
      delay(100);
      return;
    }
    Serial.println("WiFi connected");
  }

  if (!client.connected()) {
    Serial.println("PubSub not connected");
    if (client.connect("mwiclient")) {
      Serial.println("PubSub connected");
      client.subscribe(MESSAGE_WAITING_TOPIC);
    } else {
      Serial.println("PubSub failed to connect");
    }
  }

  count++;
  if (count >= 50 ) {
    count = 0;
    if ((true == messageWaiting) && (false == ledOn)) {
      digitalWrite(LED_PIN, HIGH);
      ledOn = true;
    } else {
      digitalWrite(LED_PIN, LOW);
      ledOn = false;
    }
  }
  delay(BLINK_INTERVAL_SECONDS * 1000/50);
}
