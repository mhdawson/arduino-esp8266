// Copyright 2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "WirelessConfig.h"
#include "RXCore433.h"
#include "MqttDevice2262n.h"
#include "MeatThermometer1.h"
#include "BluelineDevice.h"
#include "ArduinoTHSensor.h"
#include "Device1527.h"
#include "LacrossTX141.h"
#include "ArduinoLightSensor.h"
#include "ArduinoDS18B20Sensor.h"

#define RX_433_PIN D4

IPAddress server(mqttServer[0], mqttServer[1],
                 mqttServer[2], mqttServer[3]);

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;
PubSubClient client(wclient, server);

RXCore433 receiver(RX_433_PIN);

void setup() {
  Serial.begin(9600);
  Serial.println("Started\n");

  receiver.registerDevice(new MqttDevice2262n(200, 50, 4, &client, "esp/house/2262/200"));
  receiver.registerDevice(new MeatThermometer1(&client, "esp/house/meat/temp"));
  receiver.registerDevice(new BluelineDevice(0x1efd, &client, "esp/house/blueline"));
  receiver.registerDevice(new ArduinoTHSensor(&client, "esp/house/arduinoTHSensor"));
  receiver.registerDevice(new Device1527(350, 50, 4, &client, "esp/house/1527/350"));
  receiver.registerDevice(new LacrossTX141(&client, "esp/house/lacrossTX141"));
  receiver.registerDevice(new ArduinoLightSensor(&client, "esp/house/arduinoLightSensor"));
  receiver.registerDevice(new ArduinoDS18B20Sensor(&client, "esp/house/arduinoDS18B20Sensor"));

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



