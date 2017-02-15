#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <stdio.h>
#include "string.h"

// device specifics
#include "WirelessConfig.h"

#define OFF_PIN D5
#define HIGH_PIN D6
#define MED_PIN D7
#define LOW_PIN D8

int selectpins[] = { D0, D1, D2, D3 };

#define FAN_TOPIC "house/fan"

IPAddress server(mqttServer[0], mqttServer[1],
                 mqttServer[2], mqttServer[3]);

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;
PubSubClient client(wclient, server);

void pushButton(int pin) {
  Serial.println("sending button");
  digitalWrite(pin, HIGH);
  delay(500);
  digitalWrite(pin, LOW);
}

void callback(const MQTT::Publish& pub) {
  // we expect the message to be in the form off  "code,command" were
  // code is one of 0000 through 1111 and command is one of
  // off, low, med, high
  char* message = new char [pub.payload_string().length()+1];
  strcpy (message, pub.payload_string().c_str());
  Serial.println(message);
  char* code = strtok((char*)message, ",");
  char* command = strtok(NULL, ",");

  if ((code == NULL) || (command == NULL)) {
    Serial.println("Bad Command");
    return;
  }

  Serial.println(code);
  Serial.println(command);

  for (int i=0; i<4; i++) {
    if (code[i] == '1') {
      digitalWrite(selectpins[i], LOW);
    } else {
      digitalWrite(selectpins[i], HIGH);
    }
  }

  if(0 == strcmp(command, "off")) {
    pushButton(OFF_PIN);
  } else if(0 == strcmp(command, "low")) {
    pushButton(LOW_PIN);
  } else if(0 == strcmp(command, "med")) {
    pushButton(MED_PIN);
  } else if(0 == strcmp(command, "high")) {
    pushButton(HIGH_PIN);
  }
 }

void setup() {
  pinMode(OFF_PIN, OUTPUT);
  pinMode(LOW_PIN, OUTPUT);
  pinMode(MED_PIN, OUTPUT);
  pinMode(HIGH_PIN, OUTPUT);
  pinMode(selectpins[0], OUTPUT);
  pinMode(selectpins[1], OUTPUT);
  pinMode(selectpins[2], OUTPUT);
  pinMode(selectpins[3], OUTPUT);
    
  Serial.begin(115200);
  Serial.println("starting");
  
  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);

  client.set_callback(callback);
}


void loop() {
  client.loop();

  // make sure we are good for wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
  
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      return;
    }
    Serial.println("WiFi connected");
  }

  if (!client.connected()) {
    if (client.connect("sensclient")) {
       client.subscribe(FAN_TOPIC);
    }
  }
            
  delay(100);
}
