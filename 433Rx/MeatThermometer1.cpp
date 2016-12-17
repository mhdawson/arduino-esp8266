// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "MeatThermometer1.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEVICE_ID 12
#define DEVICE_NAME "MeatThermometer1"

#define MAX_SHORT_PULSE_LENTH 1500
#define MIN_PULSE_LENGTH 500
#define SYNC_DURATION_MIN 3800
#define SYNC_DURATION_MAX 4500
#define EXPECTED_MESSAGE_BITS 32

MeatThermometer1::MeatThermometer1() : Device() {
  syncFound = false;
  bitCount = 0;
  code = 0;
  currentBit = 0;
  syncCount = 0;
  shortCount = 0;
}

int MeatThermometer1::deviceType(void) {
  return DEVICE_ID;
};

char* MeatThermometer1::deviceName(void) {
  return (char*) DEVICE_NAME;
};

INTERRUPT_SAFE void MeatThermometer1::processPulse(long duration) {
  // any pulse less than MIN_PULSE_LENGTH means we have noise so we are not in the middle
  // of a transmision
  if (duration < MIN_PULSE_LENGTH) {
    syncFound = false;
    syncCount = 0;
    return;
  }

  // capture the next bit and decode if we are at the end of the transmission
  if (syncFound) {
    if (duration < MAX_SHORT_PULSE_LENTH) {
      // short pulse. we expect pulses in pairs, either 2 short (0) or
      // 1 short 1 long (1) so if this it the first short pulse just
      // wait until the next pulse to decide if the bit is 0 or 1.
      // If it is the second short pulse then the bit is 0
      shortCount++;
      if (shortCount == 2) {
        // 0 so don't set in code, just move to next bit
        currentBit = ((currentBit>>1) & 0x7FFFFFFF);
        shortCount = 0;
        bitCount++;
      }
    } else {
      // long pulse
      if (shortCount != 1 )  {
        // this means we have an invalid message
        syncFound = false;
        syncCount = 0;
        return;
      }

      // bit is zero so set in code and move to next bit
      code = code | currentBit;
      currentBit = ((currentBit>>1) & 0x7FFFFFFF);
      bitCount++;
      shortCount = 0;
    }

    // check if we have all 32 bits that we expect as part of the message
    if (EXPECTED_MESSAGE_BITS == bitCount) {
      unsigned char* bytes = (unsigned char*) &code;

      // this is broken out as was experimenting to see if second half
      // of checksum was fletcher's checksum which requires sumA
      // after each component is added. It does not seem to be
      // so still need to work on how to decode the other half
      unsigned char sumA = 0;
      sumA = sumA + bytes[3];
      sumA = sumA + bytes[2];
      sumA = sumA + bytes[1];

      // if the checksum is valid
      if((sumA & 0xF) == (bytes[0] & 0xF)) {
        Message* newMessage = queue->getFreeMessage();
        if (NULL != newMessage) {
          memset(newMessage, 0, sizeof(Message));
          newMessage->device = (void*) this;
          newMessage->timestamp = time(NULL);
          newMessage->code = code;
          queue->enqueueMessage(newMessage);
        } else {
          // no messages available so just drop this value
        }
      }
      syncFound = false;
    }
  } else {
    // sync consists of 2 pairs of a double long pulse followed by a short pulse
    if ((syncCount == 1)&&(duration < MAX_SHORT_PULSE_LENTH)) {
      // this is the expected short pulse after the first longer sync
      // just wait for next long pulse
    } else if ((duration > SYNC_DURATION_MIN)&&(duration < SYNC_DURATION_MAX)) {
      syncCount++;
      if (syncCount == 2) {
        code = 0;
        syncFound = true;
        syncCount = 0;
        bitCount = 0;
        currentBit = 0x80000000;
      }
    } else {
      syncFound = false;
      syncCount = 0;
    }
  }
};

void MeatThermometer1::decodeMessage(Message* message){
  char* bytes = (char*) &message->code;
  unsigned int device = bytes[3];

  // temperature is encoded as the celcius temperator multiplied by 10 and
  // is stored in bytes[2] and bytes[1]
  int intval = bytes[2]*256 + bytes[1];
  float val = ((float)intval)/10;

  message->value = val;
  #ifdef __arm__
  sprintf(message->text,"[%x] - %ld, temp: %f", device, message->timestamp, message->value
  #else
  char buffer[100];
  dtostrf(message->value, 6, 2, buffer);
  sprintf(message->text,"[%x] - %ld, temp: %s", device, message->timestamp, buffer);
  #endif
};


