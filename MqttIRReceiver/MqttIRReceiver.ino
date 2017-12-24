// Copyright 2017 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "limits.h"
#include <string>
#include "WirelessConfig.h"

#define RX_IR_PIN D5
#define DELAY_MILLIS 50
#define MAX_CODE_ENTRIES 500
#define END_TIME_VALUE 100000
#define MIN_MESSAGE_ENTRIES 20
#define IR_TOPIC "house/ir1"

static unsigned int globalEntries[MAX_CODE_ENTRIES];
static int globalCurrentEntry = 0;
static unsigned long globalLastInterruptTime;

IPAddress server(mqttServer[0], mqttServer[1],
                 mqttServer[2], mqttServer[3]);

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;

void callback(char* topic, uint8_t* message, unsigned int length) {
};
PubSubClient client(mqttServerString, mqttServerPort, callback, wclient);

ICACHE_RAM_ATTR void handleInterrupt() {

  // get the duration of the pulse
  unsigned long timeMicros = micros();
  unsigned long duration = timeMicros - globalLastInterruptTime;
  if (duration > ((unsigned int) duration)) {
    duration = INT_MAX;
  }
  globalEntries[globalCurrentEntry] = duration;
  globalCurrentEntry++;
  globalLastInterruptTime = timeMicros;
  if (globalCurrentEntry == MAX_CODE_ENTRIES -1) {
    globalCurrentEntry = 0;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("starting");

  pinMode(RX_IR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(RX_IR_PIN),&handleInterrupt,CHANGE);

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
    String clientId = String(IR_TOPIC) + String("IR_Receiver");
    if (client.connect(clientId.c_str())) {
      Serial.println("PubSub connected");
      //client.subscribe(LED_TOPIC);
    } else {
      Serial.println("PubSub failed to connect");
    }
  }

  delay(DELAY_MILLIS);

  // calculate how long we have been waiting for a change
  long lastInterruptTime = globalLastInterruptTime;
  long duration = micros() - globalLastInterruptTime;
  if ((duration > END_TIME_VALUE)&&(globalCurrentEntry != 0)) {
    unsigned int currentEntries[MAX_CODE_ENTRIES];
    int numberEntries = 0;
    bool emitMessage = false;
    noInterrupts();
    // now that we have disabled interrupts validate that there hvae been no changes since
    // we read the current value.   If there have been then we are not at the end
    // of the message or a new code is already being received so just wait until all is done
    if (lastInterruptTime == globalLastInterruptTime) {
      // copy over the values
      numberEntries = globalCurrentEntry;
      memcpy(currentEntries, globalEntries, sizeof(int)*globalCurrentEntry);
      globalCurrentEntry = 0;
      if (numberEntries > MIN_MESSAGE_ENTRIES) {
        // anything shorter is not a real message
        emitMessage = true;
      }
    }
    interrupts();
    if (emitMessage) {
      String message;
      for (int i=0; i < numberEntries; i++) {
        char buffer[sizeof(int)*2 + 1];
        message = message + String(ltoa(currentEntries[i], buffer, 16));
        if (i != numberEntries -1) {
          message = message + String(",");
        }
      }
      client.publish(IR_TOPIC, message.c_str());
    }
  }
}
