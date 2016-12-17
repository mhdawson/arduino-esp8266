// Copyright 2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "WirelessConfig.h"
#include "Device2262n.h"
#include "MqttBridge.h"
#include "RXCore433.h"

#define RX_433_PIN D4

IPAddress server(mqttServer[0], mqttServer[1],
                 mqttServer[2], mqttServer[3]);

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;
PubSubClient client(wclient, server);
MqttBridge bridge(&client, "house/esp");

RXCore433 receiver(RX_433_PIN);

void setup() {
  Serial.begin(9600);
  Serial.println("Started\n");

  Device2262n* device2262 = new Device2262n(200,50,4);
  device2262->registerMessageHandler(&bridge);
  receiver.registerDevice(device2262);

  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);
}

void loop() {
  delay(2000);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);
  
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      return;
      Serial.println("WiFi connected");
    }
  }
  
  if (!client.connected()) {
    if (client.connect("arduinoClient")) {
    }
  }

  receiver.handleMessage();

}



