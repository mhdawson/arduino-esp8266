// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "Device2262.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEVICE_ID 4
#define DEVICE_NAME "Device2262"

#define MIN_PULSE_LENGTH 300
#define SYNC_DURATION_MIN 12000
#define SYNC_DURATION_MAX 16000
#define EXPECTED_MESSAGE_BITS 24
#define ONE_PULSE_MIN_LENGTH 1000
#define MIN_REPEAT_COUNT 4

Device2262::Device2262(PubSubClient* client, char* topic) :
                         Device(), MqttDevice(client, topic) {
  syncFound = false;
  bitCount = 0;
  code = 0;
  memset(durations,0,sizeof(int)*BITS_IN_MESSAGE_2262);
  pulseCount = 0;
  repeatCount = 0;
  lastMessage = 0;
  this->registerMessageHandler(this);
};


int Device2262::deviceType(void) {
  return DEVICE_ID;
};


char* Device2262::deviceName(void) {
  return (char*) DEVICE_NAME;
};


INTERRUPT_SAFE void Device2262::processPulse(long duration) {
  // any pulse less than MIN_PULSE_LENGTH means we have noise
  // so we are not in the middle of a transmision
  if (duration < MIN_PULSE_LENGTH) {
    syncFound = false;
    return;
  }

  // capture the next bit and decode if we are at the end of the transmission
  if (syncFound) {
    pulseCount++;
    if (1 != (pulseCount%2)) {
      // we only look at every other pulse which is the one at the high level
      return;
    }

    // we are processing a message record the duration of the pulse which
    // we will use to build the overall message
    durations[bitCount] = duration;
    bitCount++;

    // ok we have all 24 pulses that we expect
    // each bit is duplicated, so although we have 24 pulses we
    // decode these to 12 bits.  The pairs are actually tri-state but we don't
    // take advantage of that for now
    if (BITS_IN_MESSAGE_2262 == bitCount) {
      for (int i=0;i<BITS_IN_MESSAGE_2262;i=i+2) {
        code = code << 1;
        if ((ONE_PULSE_MIN_LENGTH < durations[i]) &&
            (ONE_PULSE_MIN_LENGTH < durations[i+1])) {
          code = code | 0x1;
        } else if ((ONE_PULSE_MIN_LENGTH >= durations[i]) &&
                   (ONE_PULSE_MIN_LENGTH >= durations[i+1])) {
          // bit is 0, we only code devices to use high or
          // floating so this is invalid
          syncFound = false;
          return;
        } else if ((ONE_PULSE_MIN_LENGTH >= durations[i]) &&
                   (ONE_PULSE_MIN_LENGTH < durations[i+1])) {
          // floating we also consider this 0
        } else {
          // not a valid message as two bits don't agree
          syncFound = false;
          return;
        }
      }

      // make sure we have a valid code
      if (lastMessage == code) {
        repeatCount++;
        if (repeatCount >= MIN_REPEAT_COUNT) {
          // ok we are sure now it is a message as it has been repeated
          // enough times
          repeatCount = 0;
          lastMessage = 0;
          Message* newMessage = queue->getFreeMessage();
          if (NULL != newMessage) {
            memset(newMessage, 0, sizeof(Message));
            newMessage->device = (void*) this;
            newMessage->timestamp = Device::getTime();
            newMessage->code = code;
            queue->enqueueMessage(newMessage);
          } else {
            // no messages available so just drop this value
          }
        }
      } else {
        repeatCount = 0;
      }
      lastMessage = code;

      // ok wait for the next message
      syncFound = false;
    }
  } else {
    if ((duration > SYNC_DURATION_MIN) && (duration < SYNC_DURATION_MAX)) {
      code = 0;
      syncFound = true;
      bitCount = 0;
      pulseCount = 0;
    }
  }
};


void Device2262::decodeMessage(Message* message) {
  message->type = 1;
  sprintf(message->text, "%ld, code - %x", message->timestamp, message->code);
};


void Device2262::publishTopic(Message* message, char* buffer, int maxLength) {
  snprintf(&buffer[strlen(buffer)], maxLength - strlen(buffer),
           "/%x", message->code);
};
