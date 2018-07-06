// Copyright 2016-2018 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WirelessConfig.h"
#include "RXCore433.h"

#include "MqttDevice2262n.h"
#include "MeatThermometer1.h"
#include "Device1527.h"
#include "LacrossTX141.h"
#include "Device2262.h"
#include "SLTX583.h"
//#include "BluelineDevice.h"
//#include "ArduinoTHSensor.h"
//#include "ArduinoLightSensor.h"
//#include "ArduinoDS18B20Sensor.h"
//#include "ArduinoTHSensor2.h"

#define RX_433_PIN D4

char macAddress[] = "00:00:00:00:00:00";

void callback(char* topic, uint8_t* message, unsigned int length) {};

ESP8266WiFiGenericClass wifi;

#ifdef USE_CERTS
// if certs are used the following must be defined in WirelessConfig.h
//    unsigned char client_cert[] PROGMEM = {bytes in DER format};
//    unsigned int client_cert_len = 918;
//    unsigned char client_key[] PROGMEM = {bytes in DER format};
//    unsigned int client_key_len = 1193;
//
//    conversion can be done using
//    openssl x509 -in cert -out client.cert -outform DER
//    openssl rsa -in key -out client.key -outform DER
//    and then using xxd to generate the required array and lengths
//    see https://nofurtherquestions.wordpress.com/2016/03/14/making-an-esp8266-web-accessible/
//    for more detailed info
WiFiClientSecure wclient;
#else
WiFiClient wclient;
#endif

PubSubClient client(mqttServerString, mqttServerPort, callback, wclient);

RXCore433 receiver(RX_433_PIN);

void setup() {
  Serial.begin(9600);
  Serial.println("Started\n");

#ifdef USE_CERTS
  wclient.setCertificate_P(client_cert, client_cert_len);
  wclient.setPrivateKey_P(client_key, client_key_len);
#endif

  // note you can only use a subset of the devices at the same time.  This
  // is because the interrupt handlers need to be in ram and only so many of 
  // them can fit at the same time.
  receiver.registerDevice(new MqttDevice2262n(200, 75, 2, &client, "house/2262/200"));
  receiver.registerDevice(new MqttDevice2262n(350, 50, 4, &client, "house/2262/200"));
  receiver.registerDevice(new Device2262(&client, "house/2262"));
  receiver.registerDevice(new Device1527(350, 50, 4, &client, "house/1527/350"));
  receiver.registerDevice(new LacrossTX141(&client, "house/lacrossTX141"));
  receiver.registerDevice(new MeatThermometer1(&client, "house/meat/temp"));
  receiver.registerDevice(new SLTX583(&client, "house/SLTX583"));
  // receiver.registerDevice(new BluelineDevice(0x1efd, &client, "esp/house/blueline"));
  // receiver.registerDevice(new ArduinoTHSensor(&client, "esp/house/arduinoTHSensor"));
  // receiver.registerDevice(new ArduinoLightSensor(&client, "esp/house/arduinoLightSensor"));
  // receiver.registerDevice(new ArduinoDS18B20Sensor(&client, "esp/house/arduinoDS18B20Sensor"));
  // receiver.registerDevice(new ArduinoTHSensor2(&client, "esp/house/arduinoTHSensor"));
  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);

  // get the mac address to be used as a unique id for connecting to the mqtt server
  byte macRaw[6];
  WiFi.macAddress(macRaw);
  sprintf(macAddress,
          "%02.2X:%02.2X:%02.2X:%02.2X:%02.2X:%02.2X",
          macRaw[0],
          macRaw[1],
          macRaw[2],
          macRaw[3],
          macRaw[4],
          macRaw[5]);
}

void loop() {
  delay(500);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);
  
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("WiFi connected");
      return;
    }
  }

  if (!client.connected()) {
    if (client.connect(macAddress)) {
      Serial.println("mqtt connected:");
      Serial.println(macAddress);
      Serial.println("\n");
    }
  }

  receiver.handleMessage();
  client.loop();
}



