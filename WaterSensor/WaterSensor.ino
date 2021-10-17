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

#define TRANSMIT_INTERVAL_SECONDS 2
#define MILLIS_IN_SECOND 1000
#define LOOP_DELAY 100

#define MAX_MESSAGE_SIZE 100


#define PUMP_OFF HIGH
#define PUMP_ON LOW
int PUMP_PINS[]   = { D4, D0, D2, D3 };
int SENSOR_PINS[] = { D7, D1, D5, D6 };
unsigned int NUM_SENSORS = sizeof(SENSOR_PINS)/sizeof(SENSOR_PINS[0]);
#define WATER_SENSOR_PIN SENSOR_PINS[0]

void callback(char* topic, uint8_t* message, unsigned int length);

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

  /* if we are at target water level turn pump off */
  for (int i=0; i<NUM_SENSORS; i++) {
    pinMode(SENSOR_PINS[i], INPUT_PULLUP);
    pinMode(PUMP_PINS[i], OUTPUT);
    digitalWrite(PUMP_PINS[i], PUMP_OFF);
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
}

unsigned int currentFillSensor = NUM_SENSORS;
void startFill() {
  if (currentFillSensor >= NUM_SENSORS) {
    currentFillSensor = 0;    
  }
}

void callback(char* topic, uint8_t* message, unsigned int length) {
  if (strncmp((const char*)message,"on", strlen("on")) == 0) {
    startFill();
    client.publish(WATER_SENSOR_TOPIC, "water fill started");
    Serial.println("water fill started\n");
  }
};

// check if we are filling and if so drive state machine
// each bucket is filled one after the other
void checkFill() {
  if (currentFillSensor < NUM_SENSORS) {
    if (digitalRead(SENSOR_PINS[currentFillSensor]) == 1) {
      digitalWrite(PUMP_PINS[currentFillSensor], PUMP_OFF);
      currentFillSensor++;
      if (currentFillSensor >= NUM_SENSORS) {
        client.publish(WATER_SENSOR_TOPIC, "water fill complete");
        Serial.println("water fill complete\n");
      } else {
        char tempMessage[MAX_MESSAGE_SIZE];
        sprintf(tempMessage, "filling bucket: %d\n", currentFillSensor);
        client.publish(WATER_SENSOR_TOPIC, tempMessage);
        Serial.println(tempMessage);
      }
    } else {
      digitalWrite(PUMP_PINS[currentFillSensor], PUMP_ON);
    }
  }
}

void loop() {
  client.loop();
  delay(LOOP_DELAY);

  // safety check - turn all pumps off that show as full
  for (int i=0; i<NUM_SENSORS; i++) {
    if (digitalRead(SENSOR_PINS[i]) == 1) {
      digitalWrite(PUMP_PINS[i], PUMP_OFF);
    };
  };

  // check if we are filling and drive state machine
  // if we are
  checkFill();
  
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
      client.subscribe(WATER_CONTROL_TOPIC);
    }
  }
  
  counter++;
  if (counter == (TRANSMIT_INTERVAL_SECONDS * (MILLIS_IN_SECOND/LOOP_DELAY))) {
    Serial.println("Sending");

    char tempMessage[MAX_MESSAGE_SIZE];
    sprintf(tempMessage, "water state: %d\n", digitalRead(WATER_SENSOR_PIN));
    client.publish(WATER_SENSOR_TOPIC, tempMessage);
    Serial.println(tempMessage);

    counter = 0;
  }
}
