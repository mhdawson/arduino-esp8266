// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "BluelineDevice.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEVICE_ID 1
#define DEVICE_NAME "blueline"

#define SYNC_PULSE_THRESHOLD 20000
#define ONE_PULSE_MAX_LENGTH 1200
#define MIN_PULSE_LENGTH 200

BluelineDevice::BluelineDevice(long houseCode) : Device() {
   syncFound = false;
   bitCount = 0;
   code = 0;
   this->houseCode = houseCode; 
   memset(durations,0,sizeof(int)*BITS_IN_MESSAGE);
   pulseCount = 0;
}

int BluelineDevice::deviceType(void) {
   return DEVICE_ID;
};

char* BluelineDevice::deviceName(void) {
   return (char*) DEVICE_NAME;
};

INTERRUPT_SAFE void BluelineDevice::processPulse(long duration) {
   // any pulse less than MIN_PULSE_LENGTH means we have noise so we are not in the middle
   // of a transmision
   if (duration < MIN_PULSE_LENGTH) {
      syncFound = false;
      return;
   }

   if (syncFound) {
      pulseCount++;
      if (0 != (pulseCount%2)) { 
         // only the first part of the overall pulse we are looking at
         // we record the duration and then wait for the second part of the pulse
         durations[bitCount] = duration;
         return;
      }

      // we are processing a message record the duration of the pulse which 
      // we will use to build the overall message 
      durations[bitCount] += duration;
      bitCount++;
 
      // ok we have all 32 bits that we expect 
      if (BITS_IN_MESSAGE == bitCount) {
         for (int i=0; i < BITS_IN_MESSAGE; i++) {
             code = code << 1;
             if (ONE_PULSE_MAX_LENGTH > durations[i]) {
                code = code | 0x1;
             }
         }

         // make sure we have a valid code which always starts with 0xFE
	 if ((code & 0xFF000000) == 0XFE000000) {
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

         // ok message processed, indicate we are looking for the next sync
         syncFound = false;
      }
   } else {
      // not processing a message, check if the current pulse is the sync pulse
      if (duration > SYNC_PULSE_THRESHOLD) {
         syncFound = true;
         bitCount = 0;
         code = 0;
         pulseCount = 0;
      }
  }
};

void BluelineDevice::decodeMessage(Message* message){
   if ((message->code & 0x00030000) == 0x00020000) {
      // current temperature
      message->type = 1;
      message->value = (((((message->code & 0x0000FF00)-(houseCode & 0xFF00)) &0xFF00) >> 8))*0.75 - 19;
      message->value = (message->value -32)/1.8;
#ifdef __arm__
   sprintf(message->text, "%ld, %x - temp: %f", message->timestamp, message->code, message->value);
#else
   char buffer[100];
   dtostrf(message->value, 6, 2, buffer);
   sprintf(message->text,"%ld, %x - temp: %s", message->timestamp, message->code, buffer);
#endif
   } else if ((message->code & 0x00030000) == 0x00010000) {
      // current power use
      message->type = 2;
      message->value = ((float) 3600)/(((message->code & 0x0000FF00)) + ((message->code & 0x00FF0000)>>16) - houseCode);
#ifdef __arm__
      sprintf(message->text, "%ld, %x - power: %f", message->timestamp, message->code, message->value);
#else
   char buffer[100];
   dtostrf(message->value, 6, 2, buffer);
   sprintf(message->text,"%ld, %x - power: %s", message->timestamp, message->code, buffer);
#endif
   } else {
      message->type = 4;
      message->value = 0;
   }
};


void BluelineDevice::publishTopic(Message* message, char* buffer, int maxLength) {
   if (1 == message->type ) {
      strncpy(buffer, (char*) "house/blueline/temp", maxLength);
   } else if (2 == message->type) {
      strncpy(buffer, (char*) "house/blueline/power", maxLength);
   }
}
