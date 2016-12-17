// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "Device.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

Device::Device() {
  _messageHandlers = NULL;
}


void Device::setQueue(MessageQueue* queue){
  this->queue = queue;
};


int Device::numMessages(void) {
  return 1;
}


void Device::getMessageText(int messageNum, Message* message, char* buffer, int maxLength) {
  strncpy(buffer, message->text, maxLength);
}


void Device::publishTopic(int messageNum, Message* message, char* buffer, int maxLength) {
  publishTopic(message, buffer, maxLength);
}


bool Device::registerMessageHandler(DeviceMessageHandler* handler) {
  MessageHandlerListEntry* handlerEntry = (MessageHandlerListEntry*) malloc(sizeof(MessageHandlerListEntry));
  if (NULL != handlerEntry) {
    handlerEntry->handler = handler;
    handlerEntry->next = _messageHandlers;
    _messageHandlers = handlerEntry;
    return true;
  }
  return false;
}


void Device::handleMessage(Message* message) {
  // let each registerd handler have a shot at handling the message
  MessageHandlerListEntry* nextHandler = _messageHandlers;
  while(NULL != nextHandler) {
    nextHandler->handler->handleMessage(message);
    nextHandler = nextHandler->next;
  }
}


void Device::publishTopic(Message* message, char* buffer, int maxLength) {
}

