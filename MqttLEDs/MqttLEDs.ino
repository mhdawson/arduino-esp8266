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

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_PIN, NEO_GRB + NEO_KHZ800);

void callback(char* topic, uint8_t* message, unsigned int length) {
  std::string messageBuffer((const char*) message, length);
  Serial.println(messageBuffer.c_str());
  
  if (0 == strcmp(messageBuffer.c_str(), OFF)) {
    for(uint16_t i=0; i <strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
  } else if (0 == strncmp(RANGE, messageBuffer.c_str(), strlen(RANGE))) {
    char* next = strtok((char*) messageBuffer.c_str()," ");
    next = strtok(nullptr," ");
    int start = atoi(next);
    next = strtok(nullptr," ");
    int end = atoi(next);
    next = strtok(nullptr," ");
    int R = atoi(next);
    next = strtok(nullptr," ");
    int G = atoi(next);
    next = strtok(nullptr," ");
    int B = atoi(next);
    if ((start < 0)  ||  
        (start >= strip.numPixels()) ||
        (end >= strip.numPixels()) ||
        (end < 0) ||
        (end >= strip.numPixels()) ||
        (end < 0) ||
        (R > 255) ||
        (R < 0) ||
        (G > 255) ||
        (G < 0) ||
        (B > 255) ||
        (B < 0)) {
      Serial.println("Message Invalid");   
      return;    
    } 

    end = end + 1;
    for(uint16_t i = start; i < end; i++) {
      strip.setPixelColor(i, strip.Color(R, G, B));
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
