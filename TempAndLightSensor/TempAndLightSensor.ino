// Copyright the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// device specifics
#include "WirelessConfig.h"
#include "SensorConfig.h"

#define TRANSMIT_INTERVAL_SECONDS 60
#define MILLIS_IN_SECOND 1000
#define LOOP_DELAY 100

#define LED_PIN D3
#define LED_ON_TIME_SECONDS 2

#define MAX_MESSAGE_SIZE 100

#define LIGHT_PIN A0

#define DS18B20_PIN D4  // don't use D0 or D2 as can interfere with boot
OneWire ds(DS18B20_PIN);
DallasTemperature tempSensors(&ds);


void callback(char* topic, uint8_t* message, unsigned int length) {
  if (strncmp((const char*)message,"on", strlen("on")) == 0) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
};

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

int counter = 0;
char macAddress[] = "00:00:00:00:00:00";

void setup() {
  delay(1000);

  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Started");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

#ifdef USE_CERTS
  wclient.setCertificate_P(client_cert, client_cert_len);
  wclient.setPrivateKey_P(client_key, client_key_len);
#endif

  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);

  // first reading always seems to be wrong, read it early and
  // throw it away
  tempSensors.requestTemperatures();

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
      delay(100);
      return;
    }
  }

    
  if (!client.connected()) {
    if (client.connect(macAddress)) {
      Serial.println("mqtt connected:");
      Serial.println(macAddress);
      Serial.println("\n");
      client.subscribe(LED_TOPIC);
    }
  }

  counter++;
  if (counter == (TRANSMIT_INTERVAL_SECONDS * (MILLIS_IN_SECOND/LOOP_DELAY))) {
    Serial.println("Sending");

    // don't send out temperature too often as we'll get 
    // incorrect values if we sample too often
    char tempMessage[MAX_MESSAGE_SIZE];
    char floatBuffer[10];
    tempSensors.requestTemperatures();
    float currentTemp = tempSensors.getTempCByIndex(0);
    snprintf(tempMessage, MAX_MESSAGE_SIZE, "0, 0 - temp: %s",
             dtostrf(currentTemp, 4, 2, floatBuffer));
    client.publish(TEMP_TOPIC, tempMessage);

    char lightMessage[MAX_MESSAGE_SIZE];
    int lightValue = analogRead(LIGHT_PIN);
    snprintf(lightMessage, MAX_MESSAGE_SIZE, "0, 0 - light: %d", lightValue);
    client.publish(LIGHT_TOPIC, lightMessage);

    digitalWrite(LED_PIN, HIGH);
    counter = 0;
  } else if (counter == (LED_ON_TIME_SECONDS * (MILLIS_IN_SECOND/LOOP_DELAY))) {
    digitalWrite(LED_PIN, LOW);
  }
}
