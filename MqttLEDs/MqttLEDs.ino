// Copyright 2016-2017 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// device specifics
#include "WirelessConfig.h"

#define LED_PIN D6
#define LED_TOPIC "house/led"

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;

const char* OFF = "off";
const char* RANGE = "range";
const char* CLEAR_AND_RANGE = "clear+range";

boolean extractRangeAndTarget(const char* buffer,
                              int* start,
                              int* end,
                              int numPixels,
                              uint8_t* R,
                              uint8_t* G,
                              uint8_t* B) {
  *start = 0;
  *end = 0;
  *R = 0;
  *G = 0;
  *B = 0;
  char* next = strtok((char*) buffer," ");
  next = strtok(nullptr," ");
  *start = atoi(next);
  next = strtok(nullptr," ");
  *end = atoi(next);
  next = strtok(nullptr," ");
  *R = atoi(next);
  next = strtok(nullptr," ");
  *G = atoi(next);
  next = strtok(nullptr," ");
  *B = atoi(next);
  if ((*start < 0)  ||  
      (*start >= numPixels)||
      (*end >= numPixels) ||
      (*end < 0) ||
      (*end >= numPixels) ||
      (*end < 0) ||
      (*R > 255) ||
      (*R < 0) ||
      (*G > 255) ||
      (*G < 0) ||
      (*B > 255) ||
      (*B < 0)) {
    Serial.println("Message Invalid");   
    return false;    
  }             
  return true;             
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_PIN, NEO_GRB + NEO_KHZ800);

void callback(char* topic, uint8_t* message, unsigned int length) {
  std::string messageBuffer((const char*) message, length);
  Serial.println(messageBuffer.c_str());
   int start, end;
   uint8_t R, G, B;
  
  if (0 == strcmp(messageBuffer.c_str(), OFF)) {
    for(uint16_t i=0; i <strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  } else if ((0 == strncmp(RANGE, messageBuffer.c_str(), strlen(RANGE))) ||
             (0 == strncmp(CLEAR_AND_RANGE, messageBuffer.c_str(), strlen(CLEAR_AND_RANGE)))) {
    if (0 == strncmp(CLEAR_AND_RANGE, messageBuffer.c_str(), strlen(CLEAR_AND_RANGE))) {
      for(uint16_t i=0; i <strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0,0,0));
      }
    }

    if (extractRangeAndTarget(messageBuffer.c_str(), &start, &end, strip.numPixels(), &R, &G, &B)) {
      end = end + 1;
      for(uint16_t i = start; i < end; i++) {
        strip.setPixelColor(i, strip.Color(R, G, B));
      }
    }
  } else {
    Serial.println("Message Invalid");   
    return;   
  }
  
  strip.show();
};

PubSubClient client(mqttServerString, mqttServerPort, callback, wclient);

void setup() {
  strip.begin();
  for(uint16_t i=0; i <strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0,0,0));
  }
  strip.show(); // Initialize all pixels to 'off'
  
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
      client.subscribe(LED_TOPIC);
    } else {
      Serial.println("PubSub failed to connect");
    }
  }

  delay(100);
}
