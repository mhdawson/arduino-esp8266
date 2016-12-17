// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "RXCore433.h"

typedef struct DeviceListEntry {
  Device* device;
  DeviceListEntry* next;
} DeviceListEntry;

static MessageQueue* queue;
static DeviceListEntry* devices;

RXCore433::RXCore433(int interrupt) {
  pinMode(interrupt, INPUT);
  attachInterrupt(digitalPinToInterrupt(interrupt),&handleInterrupt,CHANGE);
  queue = new MessageQueue();
  devices = NULL;
  memset(&_lastHandledMessage, 0, sizeof(Message));
}

// we do most of the work outside of the interrupt handler.  The interrupt
// handler allows devices to enqueue messages and then this method processes the
// messages as time permits.
void RXCore433::handleMessage() {
  // check for new messages
  Message* currentMessages = queue->dequeueMessages();
  if (NULL != currentMessages) {
    Message* nextMessage = currentMessages;
    Message* lastMessage = NULL;
    while(NULL != nextMessage) {
      Device* messageDevice =  ((Device*) nextMessage->device);

      // do an post processing need to fill out the the message from the raw data
      messageDevice->decodeMessage(nextMessage);

      // check if this is a duplicate message, only handle if not a duplicate
      if (nextMessage->timestamp > (_lastHandledMessage.timestamp + 1) ||
          _lastHandledMessage.value != nextMessage->value ||
          _lastHandledMessage.device != nextMessage->device ||
          _lastHandledMessage.code != nextMessage->code ||
          _lastHandledMessage.type != nextMessage->type
      ) {
        // now ask the device to handle the message
        messageDevice->handleMessage(nextMessage);

        // set this message as the last message handled
        memcpy(&_lastHandledMessage, nextMessage, sizeof(Message));
      }
      lastMessage = nextMessage;
      nextMessage = nextMessage->next;
    }
    queue->returnMessages(currentMessages, lastMessage);
  }
}

ICACHE_RAM_ATTR void RXCore433::handleInterrupt() {
  static unsigned long lastInterruptTime;
  static unsigned int duration;

  // get the duration of the pulse including botht the 0 and 1 state
  long timeMicros = micros();
  duration = timeMicros - lastInterruptTime;
  lastInterruptTime = timeMicros;

  // let each registerd device process the pulse
  DeviceListEntry* nextDevice = devices;
  while(NULL != nextDevice) {
    nextDevice->device->processPulse(duration);
    nextDevice = nextDevice->next;
  }
}

bool RXCore433::registerDevice(Device* newDevice) {
  DeviceListEntry* deviceEntry = (DeviceListEntry*) malloc(sizeof(DeviceListEntry));
  if (NULL != deviceEntry) {
    newDevice->setQueue(queue);
    deviceEntry->device = newDevice;
    deviceEntry->next = devices;
    devices = deviceEntry;
    return true;
  }
  return false;
}
