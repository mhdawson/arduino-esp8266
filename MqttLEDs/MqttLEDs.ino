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
#define DELAY_MILLIS 20

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;

#define MAX_CYCLE_LENGTH 20

const char* OFF = "off";
const char* RANGE = "range";
const char* CLEAR_AND_RANGE = "clear+range";
const char* FADE = "fade";
const char* CYCLE = "cycle";
const char* FIRE = "fire";

#define FADE_COMMAND 1
#define FIRE_COMMAND 2

#define FIRE_RED 255
#define FIRE_GREEN 100
#define FIRE_BLUE 10

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
  int duration_left;
  CommandEntry* next_command;
  CommandEntry* next_in_queue;
};

CommandEntry* command_queue = nullptr;

void enqueueCommand(struct CommandEntry* command) {
  command->duration_left = command->duration;
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
    if (head->next_command != nullptr) {
      CommandEntry* next = head->next_command;
      while(next->next_command != head) {
        if (next->next_command == nullptr) {
          // not a cycle just end when we hit null
          break;
        }
        CommandEntry* current = next;
        next = next->next_command;
        delete(current);
      }
    }
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

boolean extractRange(int* start,
                     int* end,
                     int numPixels) {
  *start = 0;
  *end = 0;
  char* next = strtok(nullptr," ");
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
  return true;
}

boolean extractRangeAndTarget(int* start,
                              int* end,
                              int numPixels,
                              uint8_t* R,
                              uint8_t* G,
                              uint8_t* B) {
  if (extractRange(start, end, numPixels)) {
    return extractTarget(R, G, B);
  } 
  return false;
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_PIN, NEO_GRB + NEO_KHZ800);

void callback(char* topic, uint8_t* message, unsigned int length) {
  std::string messageBuffer((const char*) message, length);
  Serial.println(messageBuffer.c_str());
   int start, end;
   uint8_t R, G, B;
  
  if (0 == strncmp(CYCLE, messageBuffer.c_str(), strlen(CYCLE))) {
    boolean result = true;
    char* next = strtok((char*) messageBuffer.c_str()," ");
    next = strtok(nullptr," ");
    int count = atoi(next);
    if ((count < 2) || (count > MAX_CYCLE_LENGTH)) {
       Serial.println("Invalid count for cycle");
       return;
    }

    next = strtok(nullptr," ");
    int duration = atoi(next);   
        
    CommandEntry* commands[count];
    for (int i = 0; i < count; i++) {
      commands[i] = new CommandEntry;
      commands[i]->command = FADE_COMMAND;
      commands[i]->duration = duration;
    }
        
    if (extractRangeAndTarget(&commands[0]->range_start, 
                              &commands[0]->range_end, 
                              strip.numPixels(), 
                              &commands[0]->R,
                              &commands[0]->G,
                              &commands[0]->B)) {
      commands[0]->next_command = commands[1];   
      for (int i = 1; i < count; i++) {
        commands[i]->range_start = commands[0]->range_start;
        commands[i]->range_end = commands[0]->range_end;
        if (!extractTarget(&commands[i]->R,
                           &commands[i]->G,
                           &commands[i]->B)) {
          result = false;
          break;                    
        }
        if (i < (count - 1)) {
          commands[i]->next_command = commands[i + 1];
        } else {
          commands[i]->next_command = commands[0];
        }
      }
    } 

    if (result) {
      clearQueue();
      enqueueCommand(commands[0]);
      Serial.println("Cycle enqueued");
    } else {
      for (int i = 1; i < count; i++) {
        if (commands[i] != nullptr) {
          delete commands[i];  
        }
      }
    }
  } else if (0 == strncmp(FADE, messageBuffer.c_str(), strlen(FADE))) {
    CommandEntry* command = new CommandEntry;
    char* next = strtok((char*) messageBuffer.c_str()," ");
    if (extractRangeAndTarget(&command->range_start, 
                              &command->range_end, 
                              strip.numPixels(), 
                              &command->R,
                              &command->G,
                              &command->B)) {
      command->command = FADE_COMMAND;
      next = strtok(nullptr," ");
      command->duration = atoi(next);   
      command->next_command = nullptr;
      enqueueCommand(command);
      Serial.println("Fade enqueued");
    } else {
      delete command;
    }
  } else if (0 == strncmp(FIRE, messageBuffer.c_str(), strlen(FADE))) {
    CommandEntry* command = new CommandEntry;
    char* next = strtok((char*) messageBuffer.c_str()," ");

    next = strtok(nullptr," ");
    command->duration = atoi(next); 
    if (command->duration < 0) {
      Serial.println("Invalid duration"); 
      delete command;
      return;
    }

    // max dip for green for flicker
    next = strtok(nullptr," ");
    command->G = atoi(next);
    if ((command->G < 0) || (command->G > 100)) {
      Serial.println("Invalid Green dip"); 
      delete command;
      return;
    }

    // max brightness dip
    next = strtok(nullptr," ");
    command->B = atoi(next);  
    if ((command->B < 0) || (command->B > 100)) {
      Serial.println("Invalid Brightness dip"); 
      delete command;
      return;
    }
    
    if (extractRange(&command->range_start, 
                     &command->range_end, 
                     strip.numPixels())) {
      command->command = FIRE_COMMAND;
      command->next_command = nullptr;
      clearQueue();
      enqueueCommand(command);
      Serial.println("Fire enqueued");
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

    char* next = strtok((char*) messageBuffer.c_str()," ");
    if (extractRangeAndTarget(&start, &end, strip.numPixels(), &R, &G, &B)) {
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
      current_R += (((int)(command->R) - current_R) * DELAY_MILLIS)/command->duration_left;
      current_G += (((int)(command->G) - current_G) * DELAY_MILLIS)/command->duration_left;
      current_B += (((int)(command->B) - current_B) * DELAY_MILLIS)/command->duration_left;
      for(uint16_t i = command->range_start; i <= command->range_end; i++) {
        strip.setPixelColor(i, strip.Color(current_R, current_G, current_B));
      }
      strip.show();
      if ((command->duration_left - DELAY_MILLIS) > 0) {
        command->duration_left = command->duration_left - DELAY_MILLIS;
      } else {
        if (command->next_command == nullptr) {
          // we are done
          delete(dequeueHead());
        } else {
          dequeueHead();
          enqueueCommand(command->next_command);
        }
      }
    } else if (FIRE_COMMAND == command->command) {
      if ((count * DELAY_MILLIS ) > command->duration) {
        for(uint16_t i = command->range_start; i <= command->range_end; i++) {
          int brightness = random(100 - command->B,100);
          strip.setPixelColor(i, strip.Color((FIRE_RED * brightness)/100, 
                                             random(((FIRE_GREEN - command->G)*brightness)/100, (FIRE_GREEN * brightness)/100),
                                             (FIRE_BLUE * brightness)/100));
        }
        strip.show();
        count = 0;
      } else {
        count ++;
      }
    }
  }

  delay(DELAY_MILLIS);
}
