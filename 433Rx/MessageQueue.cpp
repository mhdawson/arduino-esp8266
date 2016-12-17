// Copyright 2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <Arduino.h>
#include <string.h>
#include "MessageQueue.h"

MessageQueue::MessageQueue(void) {
  // build the list of free messages
  memset(messages, 0, MAX_MESSAGES * sizeof(Message));
  for (int i=0; i < (MAX_MESSAGES - 1) ; i++) {
    messages[i].next = &messages[i+1];
  }
  messages[MAX_MESSAGES-1].next = NULL;
  freeMessages = &(messages[0]);
  newMessages = NULL;
}

ICACHE_RAM_ATTR Message* MessageQueue::getFreeMessage(void) {
  Message* newMessage = NULL;
  newMessage = freeMessages;
  if (NULL != newMessage) {
    freeMessages = newMessage->next;
  }
  return newMessage;
};

ICACHE_RAM_ATTR void MessageQueue::enqueueMessage(Message* message) {
  message->next = newMessages;
  newMessages = message;
};

Message* MessageQueue::dequeueMessages(void) {
  Message* currentMessages = NULL;
  noInterrupts();
  currentMessages = newMessages;
  if (NULL != currentMessages) {
    newMessages = NULL;
  }
  interrupts();
  return currentMessages;
};

void MessageQueue::returnMessages(Message* messages, Message* lastMessage) {
  noInterrupts();
  lastMessage->next = freeMessages;
  freeMessages = messages;
  interrupts();
};
