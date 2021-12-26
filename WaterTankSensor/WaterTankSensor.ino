// Copyright the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// device specifics
#include "WirelessConfig.h"
#include "SensorConfig.h"
#include "ESP8266WebServer.h"

#define TRANSMIT_INTERVAL_SECONDS 2
#define MILLIS_IN_SECOND 1000
#define LOOP_DELAY 100
#define NUM_FULL_INTERVALS 30

#define MAX_MESSAGE_SIZE 100
#define MAX_TOPIC_SIZE 100


int SENSOR_PINS[] = { D1 };
unsigned int NUM_SENSORS = sizeof(SENSOR_PINS)/sizeof(SENSOR_PINS[0]);

void callback(char* topic, uint8_t* message, unsigned int length);

ESP8266WiFiGenericClass wifi;
ESP8266WebServer server(80);

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

int counter = 0;
int fullIntervalCount = 0;
char macAddress[] = "00:00:00:00:00:00";

void provideMetrics() {
  char tempMessage[MAX_MESSAGE_SIZE];
  String ptr = "";
  for (int i=0; i<NUM_SENSORS; i++) {
      int sensorValue = digitalRead(SENSOR_PINS[i]);
      snprintf(tempMessage, MAX_MESSAGE_SIZE, "Tank status%d %d\n", i, sensorValue);
      ptr+= tempMessage;
  }
  server.send(200, "text/plain", ptr);
}

void setup() {
  delay(1000);

  /* if we are at target water level turn pump off */
  for (int i=0; i<NUM_SENSORS; i++) {
    pinMode(SENSOR_PINS[i], INPUT_PULLUP);
  }

  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Started");

#ifdef USE_CERTS
  wclient.setCertificate_P(client_cert, client_cert_len);
  wclient.setPrivateKey_P(client_key, client_key_len);
#endif

  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

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

  server.on("/metrics", provideMetrics);
  server.begin();
}

void callback(char* topic, uint8_t* message, unsigned int length) {
};

void loop() {
  client.loop();
  delay(LOOP_DELAY);

  // make sure we are good for wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.reconnect();

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Failed to reconnect WIFI");
      Serial.println(WiFi.waitForConnectResult());
      delay(1000);
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

  counter++;
  if (counter == (TRANSMIT_INTERVAL_SECONDS * (MILLIS_IN_SECOND/LOOP_DELAY))) {
    Serial.println("Sending");

    char tempMessage[MAX_MESSAGE_SIZE];
    char tempTopic[MAX_TOPIC_SIZE];

    for (int i=0; i<NUM_SENSORS; i++) {
      int sensorValue = digitalRead(SENSOR_PINS[i]);
      if ((sensorValue == 0) || (fullIntervalCount >= (NUM_FULL_INTERVALS-1))) {
        sprintf(tempMessage, "%d", sensorValue);
        sprintf(tempTopic, "%s/%d", WATER_SENSOR_TOPIC, i);
        client.publish(tempTopic, tempMessage);
        fullIntervalCount = 0;
      } else {
        fullIntervalCount++;
      }
    };
    counter = 0;
  }

  server.handleClient();
}
