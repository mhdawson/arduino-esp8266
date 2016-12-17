// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "ArduinoTHSensor.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEVICE_ID 7
#define DEVICE_NAME "ArduinoTHSensor"

#define MIN_PULSE_LENGTH 100
#define SYNC_DURATION_MIN 700
#define SYNC_DURATION_MAX 1200
#define EXPECTED_MESSAGE_BITS 32
#define ONE_PULSE_MIN_LENGTH 450

ArduinoTHSensor::ArduinoTHSensor(PubSubClient* client, char* topic) : MqttDevice(client, topic) {
   syncCount = 0;
   syncFound = false;
   bitCount = 0;
   code = 0;
   this->houseCode = houseCode; 
   memset(durations,0,sizeof(int)*BITS_IN_MESSAGE);
   pulseCount = 0;
}

int ArduinoTHSensor::deviceType(void) {
   return DEVICE_ID;
};

char* ArduinoTHSensor::deviceName(void) {
   return (char*) DEVICE_NAME;
};

INTERRUPT_SAFE void ArduinoTHSensor::processPulse(long duration) {
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

         if (validateChecksum(code)) { 
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

void ArduinoTHSensor::decodeMessage(Message* message) {
   char* bytes = (char*) &message->code;
   message->type = 1;
   sprintf(message->text, "%ld, %x - temp: %d,humidity: %d",
           message->timestamp,
           message->code,
           (int) bytes[1] - 128,
           (int) bytes[2] );
};

int ArduinoTHSensor::numMessages(void) {
  return 2;
}

void ArduinoTHSensor::publishTopic(int messageNum, Message* message, char* buffer, int maxLength) {
   char tempBuffer[20];
   char* bytes = (char*) &message->code;
   if (messageNum == 0) {
      snprintf(tempBuffer, 20, "/%x/temp", bytes[3]);
   } else {
      snprintf(tempBuffer, maxLength, "/%x/humidity", bytes[3]);
   }
   strncat(buffer, tempBuffer, maxLength);
}

void ArduinoTHSensor::getMessageText(int messageNum, Message* message, char* buffer, int maxLength) {
   char* bytes = (char*) &message->code;
   if (messageNum == 0) {
      sprintf(buffer, "%ld, %x - temp: %d",
              message->timestamp,
              message->code,
              (int) bytes[1] - 128);
   } else {
      sprintf(buffer, "%ld, %x - humidity: %d",
              message->timestamp,
              message->code,
              (int) bytes[2]);
   }
}

// to calculate the checksum first reverse the nibbles in each byte
// including the checksum 
// then add the 3 message bytes together and add 0x77 to get the 
// expected checksum which is in byte 0 
bool ArduinoTHSensor::validateChecksum(int code) {
   char* bytes = (char*) & code;
   char calcChecksum = 0x77;
   char checksum = ((bytes[0] >> 4) & 0x0F) + ((bytes[0] & 0x0F) << 4);

   for (int i = 1;i<4;i++) {
      calcChecksum += ((bytes[i] >> 4) & 0x0F) + ((bytes[i] & 0x0F) << 4);
   }

   if (calcChecksum == checksum) {
      return true;
   }

   return false;
}

