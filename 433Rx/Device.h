// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _DEVICE
#define _DEVICE

#include "MessageQueue.h"
#include "DeviceMessageHandler.h"


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

    // can optionally be overridden by devices
    virtual void handleMessage(Message* message);
    virtual bool registerMessageHandler(DeviceMessageHandler* handler);
};

#endif
