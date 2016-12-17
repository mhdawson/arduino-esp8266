// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "Device2262n.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#define DEVICE_NAME "Device2262n"
#define DEVICE_ID 5

#define MIN_PULSE_LENGTH          (_pulseWidth - _pulseSlack)
#define SYNC_DURATION_MIN         (35 * MIN_PULSE_LENGTH)
#define SYNC_DURATION_MAX         (45 * _pulseWidth)
#define EXPECTED_MESSAGE_BITS     24
#define ONE_PULSE_MIN_LENGTH      ((_pulseWidth - _pulseSlack) * 3)


Device2262n::Device2262n(int pulseWidth, int pulseSlack, int minRepeats) : Device() {
  _pulseWidth = pulseWidth;
  _pulseSlack = pulseSlack;
  _minRepeats = minRepeats;
  syncFound = false;
  bitCount = 0;
  memset(durations, 0, sizeof(int)*BITS_IN_MESSAGE_2262);
  memset(tristateCode, 0, (TRISTATE_MESSAGE_LENGTH));
  pulseCount = 0;
  repeatCount = 0;
  memset(lastMessage, 0, (TRISTATE_MESSAGE_LENGTH));
};


char* Device2262n::deviceName(void) {
  return (char*) DEVICE_NAME;
};


int Device2262n::deviceType(void) {
  return DEVICE_ID;
};


INTERRUPT_SAFE void Device2262n::processPulse(long duration) {
  // any pulse less than MIN_PULSE_LENGTH means we have noise so we are not in the middle
  // of a transmision
  if (duration < MIN_PULSE_LENGTH) {
    syncFound = false;
    return;
  }

  // capture the next bit and decode if we are at the end of the transmission
  if (syncFound) {
    pulseCount++;
    if (1 != (pulseCount % 2)) {
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
      for (int i = 0; i < BITS_IN_MESSAGE_2262; i = i + 2) {
        if ((ONE_PULSE_MIN_LENGTH < durations[i]) && (ONE_PULSE_MIN_LENGTH < durations[i + 1])) {
          tristateCode[i / 2] = '1';
        } else if ((ONE_PULSE_MIN_LENGTH >= durations[i]) && (ONE_PULSE_MIN_LENGTH >= durations[i + 1])) {
          tristateCode[i / 2] = '0';
        } else if ((ONE_PULSE_MIN_LENGTH >= durations[i]) && (ONE_PULSE_MIN_LENGTH < durations[i + 1])) {
          tristateCode[i / 2] = 'F';
        } else {
          // not a valid message as two bits don't agree
          syncFound = false;
          return;
        }
      }

      // make sure we have a valid code
      if (0 == memcmp(lastMessage, tristateCode, TRISTATE_MESSAGE_LENGTH)) {
        repeatCount++;
        if (repeatCount >= _minRepeats) {
          // ok we are sure now it is a message as it has been repeated
          // enough times
          repeatCount = 0;
          memset(lastMessage, 0, TRISTATE_MESSAGE_LENGTH);
          Message* newMessage = queue->getFreeMessage();
          if (NULL != newMessage) {
            memset(newMessage, 0, sizeof(Message));
            newMessage->device = (void*) this;
            newMessage->timestamp = millis() / 1000;
            strncpy(newMessage->text, (const char*) tristateCode, TRISTATE_MESSAGE_LENGTH);
            newMessage->code = 0;
            queue->enqueueMessage(newMessage);
          } else {
            // no messages available so just drop this value
          }
        }
      } else {
        repeatCount = 0;
      }
      memcpy(lastMessage, tristateCode, TRISTATE_MESSAGE_LENGTH);
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


void Device2262n::decodeMessage(Message* message) {
  // message is already in the right format
};


