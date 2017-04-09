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
#define DELAY_MILLIS 50

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;

const char* OFF = "off";
const char* RANGE = "range";
const char* CLEAR_AND_RANGE = "clear+range";
const char* FADE = "fade";

#define FADE_COMMAND 1

uint8_t current_R = 0;
uint8_t current_G = 0;
uint8_t current_B = 0;
 
void setCurrent(uint8_t R, uint8_t G, int8_t B) {
  current_R = R;
  current_B = B;
  current_B = G;
}

struct CommandEntry {
  int command;
  int range_start;
  int range_end;
  uint8_t R;
  uint8_t G;
  uint8_t B;
  int duration;
  CommandEntry* next_command;
  CommandEntry* next_in_queue;
};

CommandEntry* command_queue = nullptr;

void enqueueCommand(struct CommandEntry* command) {
  command->next_in_queue = nullptr;
  if (command_queue == nullptr) {
    command_queue = command;
    return;
  }
  CommandEntry* tail = command_queue;
  while(tail->next_in_queue != nullptr) {
    tail = tail->next_in_queue;
  }
  tail->next_in_queue = command;
}

CommandEntry* dequeueHead() {
  if (nullptr != command_queue) {
    CommandEntry* head = command_queue;
    command_queue = command_queue->next_in_queue;
    return head;
  }
  return nullptr;
}

void clearQueue() {
  CommandEntry* head = dequeueHead();
  while(head != nullptr) {
    delete(head);
    head = dequeueHead();
  }
}

boolean extractTarget(uint8_t* R,
                      uint8_t* G,
                      uint8_t* B) {
  *R = 0;
  *G = 0;
  *B = 0;
  char* next = strtok(nullptr," ");
  *R = atoi(next);
  next = strtok(nullptr," ");
  *G = atoi(next);
  next = strtok(nullptr," ");
  *B = atoi(next);
  if ((*R > 255) ||
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

boolean extractRangeAndTarget(const char* buffer,
                              int* start,
                              int* end,
                              int numPixels,
                              uint8_t* R,
                              uint8_t* G,
                              uint8_t* B) {
  *start = 0;
  *end = 0;
  char* next = strtok((char*) buffer," ");
  next = strtok(nullptr," ");
  *start = atoi(next);
  next = strtok(nullptr," ");
  *end = atoi(next);
  if ((*start < 0)  ||  
      (*start >= numPixels)||
      (*end >= numPixels) ||
      (*end < 0) ||
      (*end >= numPixels) ||
      (*end < 0)) {
    Serial.println("Message Invalid");
    return false;    
  }             
  return extractTarget(R, G, B);
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_PIN, NEO_GRB + NEO_KHZ800);

void callback(char* topic, uint8_t* message, unsigned int length) {
  std::string messageBuffer((const char*) message, length);
  Serial.println(messageBuffer.c_str());
   int start, end;
   uint8_t R, G, B;
  
  if (0 == strncmp(FADE, messageBuffer.c_str(), strlen(FADE))) {
    CommandEntry* command = new CommandEntry;
    if (extractRangeAndTarget(messageBuffer.c_str(), 
                              &command->range_start, 
                              &command->range_end, 
                              strip.numPixels(), 
                              &command->R,
                              &command->G,
                              &command->B)) {
      command->command = FADE_COMMAND;
      char* next = strtok(nullptr," ");
      command->duration = atoi(next);   
      command->next_command = nullptr;                       
      enqueueCommand(command);
      Serial.println("Fade enqueued");
    } else {
      delete command;
    }
  } else if (0 == strcmp(messageBuffer.c_str(), OFF)) {
    clearQueue();
    for(uint16_t i=0; i <strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
    setCurrent(0, 0, 0);
  } else if ((0 == strncmp(RANGE, messageBuffer.c_str(), strlen(RANGE))) ||
             (0 == strncmp(CLEAR_AND_RANGE, messageBuffer.c_str(), strlen(CLEAR_AND_RANGE)))) {
    clearQueue();
    if (0 == strncmp(CLEAR_AND_RANGE, messageBuffer.c_str(), strlen(CLEAR_AND_RANGE))) {
      for(uint16_t i=0; i <strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0,0,0));
      }
      setCurrent(0, 0, 0);
    }

    if (extractRangeAndTarget(messageBuffer.c_str(), &start, &end, strip.numPixels(), &R, &G, &B)) {
      end = end + 1;
      for(uint16_t i = start; i < end; i++) {
        strip.setPixelColor(i, strip.Color(R, G, B));
      }
      setCurrent(R, G, B);
    }
  } else {
    Serial.println("Message Unknown");
    return;   
  }
  
  strip.show();
};

PubSubClient client(mqttServerString, mqttServerPort, callback, wclient);

void setup() {
  command_queue = nullptr;
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

  if (command_queue) {
    CommandEntry* command = command_queue;
    if (FADE_COMMAND == command->command) {
      current_R += (((int)(command->R) - current_R) * DELAY_MILLIS)/command->duration;
      current_G += (((int)(command->G) - current_G) * DELAY_MILLIS)/command->duration;
      current_B += (((int)(command->B) - current_B) * DELAY_MILLIS)/command->duration;
      for(uint16_t i = command->range_start; i <= command->range_end; i++) {
        strip.setPixelColor(i, strip.Color(current_R, current_G, current_B));
      }
      strip.show();
      if ((command->duration - DELAY_MILLIS) > 0) {
        command->duration = command->duration - DELAY_MILLIS;
      } else {
        // we are done
        delete(dequeueHead());
      }
    }
  }

  delay(DELAY_MILLIS);
}
