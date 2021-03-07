// Copyright the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Sodaq_SHT2x.h>

// device specifics
#include "WirelessConfig.h"
#include "SensorConfig.h"
#include "ESP8266WebServer.h"

#define TRANSMIT_INTERVAL_SECONDS 30
#define MILLIS_IN_SECOND 1000
#define LOOP_DELAY 100

#define MAX_MESSAGE_SIZE 100

#define LIGHT_PIN A0

void callback(char* topic, uint8_t* message, unsigned int length) {
};

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
char macAddress[] = "00:00:00:00:00:00";


char tempMessage[MAX_MESSAGE_SIZE];
char floatBuffer[10];
float currentTemp = 0;
float currentHumidity = 0;
int lightValue = 0;
void provideMetrics() {
  snprintf(tempMessage, MAX_MESSAGE_SIZE, "plant_humidity %s\n",
             dtostrf(currentHumidity, 4, 2, floatBuffer));
  String ptr = tempMessage;
  snprintf(tempMessage, MAX_MESSAGE_SIZE, "plant_temperature %s\n",
           dtostrf(currentTemp,4, 2, floatBuffer));
  ptr+= tempMessage;
  snprintf(tempMessage, MAX_MESSAGE_SIZE, "plant_light %d\n", lightValue);
  ptr+= tempMessage;

  server.send(200, "text/plain", ptr);
}

void setup() {
  delay(1000);
  Wire.begin();

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

    // don't send out temperature too often as we'll get 
    // incorrect values if we sample too often


    // read temperature in Celsius
    currentTemp = SHT2x.GetTemperature();
    snprintf(tempMessage, MAX_MESSAGE_SIZE, "0, 0 - temp: %s",
             dtostrf(currentTemp, 4, 2, floatBuffer));
    client.publish(TEMP_TOPIC, tempMessage);

    // read humidity
    currentHumidity = SHT2x.GetHumidity();
    snprintf(tempMessage, MAX_MESSAGE_SIZE, "0, 0 - hum: %s",
             dtostrf(currentHumidity, 4, 2, floatBuffer));
    client.publish(HUM_TOPIC, tempMessage);

    char lightMessage[MAX_MESSAGE_SIZE];
    lightValue = analogRead(LIGHT_PIN);
    snprintf(lightMessage, MAX_MESSAGE_SIZE, "0, 0 - light: %d", lightValue);
    client.publish(LIGHT_TOPIC, lightMessage);

    counter = 0;
  }

  server.handleClient();
}
