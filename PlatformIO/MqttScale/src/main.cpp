// Copyright the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
//#include <OneWire.h>
#include <HX711.h>

// device specifics
#include "WirelessConfig.h"
#include "MqttScale.h"

#define TRANSMIT_INTERVAL_SECONDS 5
#define MILLIS_IN_SECOND 1000
#define LOOP_DELAY 100

#define MAX_MESSAGE_SIZE 100

HX711 scale;

void callback(char* topic, uint8_t* message, unsigned int length) {
  if (strcmp(topic, SCALE_TARE_TOPIC) == 0) {
    scale.tare();
  }
}

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

// scale parameters 
int SCALE_DOUT = D2;
int SCALE_SCK = D3;
int SCALE_READINGS = 20;
int BUTTON_PIN = D5;
int MIN_BUTTON_TIME_MICROS = 250000;

ICACHE_RAM_ATTR void handleButton() {
  static unsigned long lastInterruptTime = 0;

  long timeMicros = micros();
  if (digitalRead(BUTTON_PIN) == LOW) {
    // get the duration of the button press and only accept as button
    // press if it is long enough that we know it is not noise.
    unsigned long duration = timeMicros - lastInterruptTime;
    if (duration > MIN_BUTTON_TIME_MICROS) {
      scale.tare();
    }
  }

  lastInterruptTime = timeMicros;
}

void setup() {
  delay(1000);

  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Started");

  // setup the scale
  scale.begin(SCALE_DOUT, SCALE_SCK);
  scale.set_scale(SCALE_MULTIPLIER);
  scale.set_offset(0);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),&handleButton,CHANGE);

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
      client.subscribe(SCALE_TARE_TOPIC);
    }
    scale.tare();
  }

  counter++;
  if (counter == (TRANSMIT_INTERVAL_SECONDS * (MILLIS_IN_SECOND/LOOP_DELAY))) {
    Serial.println("Sending");

    char tempMessage[MAX_MESSAGE_SIZE];
    char floatBuffer[10];
    Serial.println(scale.get_units(SCALE_READINGS), 2);
    float currentWeight = scale.get_units(SCALE_READINGS);
    snprintf(tempMessage, MAX_MESSAGE_SIZE, "%s",
             dtostrf(currentWeight, 4, 2, floatBuffer));
    client.publish(SCALE_TOPIC, tempMessage);

    counter = 0;
  }
}