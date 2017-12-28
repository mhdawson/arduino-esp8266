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
#define MIN_PULSE_LENGTH 400
#define MIN_START_MARK_LENGTH 1000
#define IR_TOPIC "house/ir1"

static int globalCurrentValue = 0;
static unsigned long globalEntries[MAX_CODE_ENTRIES];
static volatile int globalCurrentEntry = 0;
static volatile unsigned long globalLastInterruptTime;

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;

void callback(char* topic, uint8_t* message, unsigned int length) {
};
PubSubClient client(mqttServerString, mqttServerPort, callback, wclient);

ICACHE_RAM_ATTR void handleInterrupt() {
  // validate that we have actually changed state
  int lastValue = globalCurrentValue;
  globalCurrentValue = digitalRead(RX_IR_PIN);
  if (lastValue == globalCurrentValue) {
    // value has not changed so don't do anything
    return;
  };

  unsigned long timeMicros = micros();
  unsigned long duration = timeMicros - globalLastInterruptTime;
  globalEntries[globalCurrentEntry] = duration;
  globalCurrentEntry++;
  globalLastInterruptTime = timeMicros;
  if (globalCurrentEntry == MAX_CODE_ENTRIES -1) {
    globalCurrentEntry = 0;
  }
}

void setup() {
  pinMode(RX_IR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RX_IR_PIN),&handleInterrupt,CHANGE);

    // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);

  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
  }
}

int count = 0;

void loop() {
  client.loop();

  // make sure we are good for wifi
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      delay(100);
      return;
    }
  }

  if (!client.connected()) {
    String clientId = String(IR_TOPIC) + String("IR_Receiver");
    if (client.connect(clientId.c_str())) {
    } else {
    }
  }

  delay(DELAY_MILLIS);

  // calculate how long we have been waiting for a change
  long lastInterruptTime = globalLastInterruptTime;
  long duration = micros() - globalLastInterruptTime;
  if ((duration > END_TIME_VALUE)&&(globalCurrentEntry != 0)) {
    unsigned long currentEntries[MAX_CODE_ENTRIES];
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
      emitMessage = true;
     }
    interrupts();
    
    if (emitMessage) {
      // eliminate short pulses as they are just noise
      int j = 0;
      for (int i = 0; i < numberEntries; i++) {
        if ((currentEntries[i] > MIN_PULSE_LENGTH) || (j == 0 ) || ((i + 1) >= numberEntries)) {
          currentEntries[j] = currentEntries[i];
          j++;
        } else {
          currentEntries[j-1] = currentEntries[j-1] + currentEntries[i] + currentEntries[i+1];
          i++;
        }
      }
      numberEntries = j;

      if (numberEntries < MIN_MESSAGE_ENTRIES) {
        // anything shorter is not a real message
        return;
      }

      String message;
      for (int i = 0; i < numberEntries; i++) {
        char buffer[sizeof(int)*2 + 1];
        message = message + String(ultoa(currentEntries[i], buffer, 16));
        if (i != numberEntries -1) {
          message = message + String(",");
        }
      }
      client.publish(IR_TOPIC, message.c_str());
    }
  }
}
