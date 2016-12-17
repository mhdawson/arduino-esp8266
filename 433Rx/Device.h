// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _DEVICE
#define _DEVICE

#include "MessageQueue.h"
#include "DeviceMessageHandler.h"

#ifdef __arm__
#define INTERRUPT_SAFE
#else
#include <Arduino.h>
#ifdef ESP8266
#define INTERRUPT_SAFE ICACHE_RAM_ATTR
#else
#define INTERRUPT_SAFE
#endif
#endif

#define TOPIC_SEPARATOR "/"

typedef struct MessageHandlerListEntry {
  DeviceMessageHandler* handler;
  MessageHandlerListEntry* next;
} MessageHandlerListEntry;

class Device {
  protected:
    MessageQueue* queue;
    MessageHandlerListEntry* _messageHandlers;

  public:
    Device();
    void setQueue(MessageQueue* queue);

    // must be implemented by sub class
    virtual int deviceType(void) = 0;
    virtual char* deviceName(void) = 0;
    virtual void processPulse(long duration) = 0;
    virtual void decodeMessage(Message* message) = 0;

    // can be overriden if the device has more than one message
    virtual int numMessages(void);
    virtual void getMessageText(int messageNum, Message* message, char* buffer, int maxLength);
    virtual void publishTopic(int messageNum, Message* message, char* buffer, int maxLength);

    // can optionally be overridden by devices
    virtual void handleMessage(Message* message);
    virtual bool registerMessageHandler(DeviceMessageHandler* handler);
    virtual void publishTopic(Message* message, char* buffer, int maxLength);
};

#endif
