// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "Device1527.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEVICE_ID 5
#define DEVICE_NAME "Device1527"

#define MIN_PULSE_LENGTH          (_pulseWidth - _pulseSlack)
#define SYNC_DURATION_MIN         (35 * MIN_PULSE_LENGTH)
#define SYNC_DURATION_MAX         (45 * _pulseWidth)
#define EXPECTED_MESSAGE_BITS     24
#define ONE_PULSE_MIN_LENGTH      ((_pulseWidth - _pulseSlack) * 3)

Device1527::Device1527(int pulseWidth, int pulseSlack, int minRepeats,
                       PubSubClient* client, char* topic) :
                         Device(), MqttDevice(client, topic) {
  _pulseWidth = pulseWidth;
  _pulseSlack = pulseSlack;
  _minRepeats = minRepeats;
  syncFound = false;
  bitCount = 0;
  memset(durations, 0, sizeof(int)*BITS_IN_MESSAGE_1527);
  memset(receivedCode, 0, sizeof(unsigned char)*(BITS_IN_MESSAGE_1527+1));
  pulseCount = 0;
  repeatCount = 0;
  memset(lastMessage, 0, sizeof(lastMessage));
  this->registerMessageHandler(this);
}

int Device1527::deviceType(void) {
  return DEVICE_ID;
};

char* Device1527::deviceName(void) {
  return (char*) DEVICE_NAME;
};

INTERRUPT_SAFE void Device1527::processPulse(long duration) {
  // any pulse less than MIN_PULSE_LENGTH means we have
  // noise so we are not in the middle of a transmision
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
    if (BITS_IN_MESSAGE_1527 == bitCount) {
      for (int i=0;i<BITS_IN_MESSAGE_1527;i=i+1) {
        if (ONE_PULSE_MIN_LENGTH < durations[i]) {
          receivedCode[i] = '1';
        } else {
          receivedCode[i] = '0';
        }
      }

      // make sure we have a valid code
      if (0 == memcmp(lastMessage, receivedCode, sizeof(lastMessage))) {
        repeatCount++;
        if (repeatCount >= _minRepeats) {
          // ok we are sure now it is a message as it has been repeated
          // enough times
          repeatCount = 0;
          memset(lastMessage,0,sizeof(lastMessage));
          Message* newMessage = queue->getFreeMessage();
          if (NULL != newMessage) {
            memset(newMessage, 0, sizeof(Message));
            newMessage->device = (void*) this;
            newMessage->timestamp = Device::getTime();
            strncpy(newMessage->text, (const char*) receivedCode, sizeof(receivedCode));
            newMessage->code = 0;
            queue->enqueueMessage(newMessage);
          } else {
            // no messages available so just drop this value
          }
        }
      } else {
        repeatCount = 0;
      }
      memcpy(lastMessage, receivedCode, sizeof(lastMessage));
      // ok wait for the next message
      syncFound = false;
    }
  } else {
    if ((duration > SYNC_DURATION_MIN) && (duration < SYNC_DURATION_MAX)) {
      syncFound = true;
      bitCount = 0;
      pulseCount = 0;
    }
  }
};

void Device1527::decodeMessage(Message* message) {
  // message is already in the right format
};

void Device1527::publishTopic(int messageNum, Message* message,
                              char* buffer, int maxLength) {
  strncat(buffer, TOPIC_SEPARATOR, maxLength);
  strncat(buffer, message->text, maxLength);
}
