// Copyright 2016-2017 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "IRSender.h"

// device specifics
#include "WirelessConfig.h"

#define IR_PIN D2
#define IR_TOPIC "house/ir"

IRSender irSender(IR_PIN);

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;

void callback(char* topic, uint8_t* message, unsigned int length) {
  std::string messageBuffer((const char*) message, length);

  Serial.println(messageBuffer.c_str());
  irSender.sendIrCode((char*) messageBuffer.c_str());
}

PubSubClient client(mqttServerString, mqttServerPort, callback, wclient);

void setup() {
  pinMode(IR_PIN, OUTPUT);
  digitalWrite(IR_PIN, LOW);

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
    if (client.connect("irclient")) {
      Serial.println("PubSub connected");
      client.subscribe(IR_TOPIC);
    } else {
      Serial.println("PubSub failed to connect");
    }
  }

  delay(100);
}
