// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "LacrossTX141.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEVICE_ID 3
#define DEVICE_NAME "LacrossTX141"

#define MIN_PULSE_LENGTH 100
#define SYNC_DURATION_MIN 700
#define SYNC_DURATION_MAX 1200
#define EXPECTED_MESSAGE_BITS 32
#define ONE_PULSE_MIN_LENGTH 450

LacrossTX141::LacrossTX141(PubSubClient* client, char* topic) : Device(), MqttDevice(client, topic) {
   syncCount = 0;
   syncFound = false;
   bitCount = 0;
   code = 0;
   this->houseCode = houseCode; 
   memset(durations,0,sizeof(int)*BITS_IN_MESSAGE);
   pulseCount = 0;
   this->registerMessageHandler(this);
}

int LacrossTX141::deviceType(void) {
   return DEVICE_ID;
};

char* LacrossTX141::deviceName(void) {
   return (char*) DEVICE_NAME;
};

INTERRUPT_SAFE void LacrossTX141::processPulse(long duration) {
   // any pulse less than MIN_PULSE_LENGTH means we have noise so we are not in the middle
   // of a transmision
   if (duration < MIN_PULSE_LENGTH) {
      syncFound = false;
      syncCount = 0;
      return;
   }

   // capture the next bit and decode if we are at the end of the transmission
   if (syncFound) {
      pulseCount++;
      if (0 != (pulseCount%2)) { 
	 // we only look at every other pulse which is the one at the high level 
         return;
      }

      // we are processing a message record the duration of the pulse which 
      // we will use to build the overall message 
      durations[bitCount] = duration;
      bitCount++;
 
      // ok we have all 32 bits that we expect 
      if (BITS_IN_MESSAGE == bitCount) {
         for (int i=0; i < BITS_IN_MESSAGE; i++) {
             code = code << 1;
             if (ONE_PULSE_MIN_LENGTH > durations[i]) {
                code = code | 0x1;
             }
         }

         // make sure we have a valid code which always starts with 0xED
         if (validateChecksum(code)) { 
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

        // ok wait for the next message
        syncFound = false;
      } 
   } else {
      if ((duration > SYNC_DURATION_MIN) && (duration < SYNC_DURATION_MAX)) {
         syncCount++;
         if (syncCount == 10 ) {
            code = 0;
            syncFound = true;
            syncCount = 0;
            bitCount = 0;
            pulseCount = 0;
         }
      } else {
         syncCount = 0;
      }
   }
};

void LacrossTX141::decodeMessage(Message* message) {
   char* bytes = (char*) &message->code;
   message->type = 1;
   message->value = ((float)((bytes[1] + (bytes[2] & 0x0F)*256) - 500))/10;

#ifdef __arm__
    snprintf(message->text, MAX_MESSAGE_TEXT_LENGTH, "%ld, %x - temp: %f", message->timestamp, message->code, message->value);
#else
    char buffer[100];
    dtostrf(message->value, 6, 2, buffer);
    snprintf(message->text, MAX_MESSAGE_TEXT_LENGTH, "%ld, %x - temp: %s", message->timestamp, message->code, buffer);
#endif
   
};

void LacrossTX141::publishTopic(Message* message, char* buffer, int maxLength) {
   char* bytes = (char*) &message->code;
   snprintf(&buffer[strlen(buffer)], maxLength-strlen(buffer), "/%x/temp", bytes[3]);
}

// to calculate the checksum first reverse the nibbles in each byte
// including the checksum 
// then add the 3 message bytes together and add 0x66 to get the 
// expected checksum which is in byte 0 
INTERRUPT_SAFE bool LacrossTX141::validateChecksum(int code) {
   char* bytes = (char*) & code;
   char calcChecksum = 0x66;
   char checksum = ((bytes[0] >> 4) & 0x0F) + ((bytes[0] & 0x0F) << 4);

   for (int i = 1;i<4;i++) {
      calcChecksum += ((bytes[i] >> 4) & 0x0F) + ((bytes[i] & 0x0F) << 4);
   }

   if (calcChecksum == checksum) {
      return true;
   }

   return false;
}

