// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "ArduinoLightSensor.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <algorithm>

#define DEVICE_ID 10
#define DEVICE_NAME "ArduinoLightSensor"

#define MIN_PULSE_LENGTH 100
#define SYNC_DURATION_MIN 700
#define SYNC_DURATION_MAX 1200
#define EXPECTED_MESSAGE_BITS 32
#define ONE_PULSE_MIN_LENGTH 450

ArduinoLightSensor::ArduinoLightSensor(PubSubClient* client, char* topic) : Device(), MqttDevice(client, topic) {
   syncCount = 0;
   syncFound = false;
   bitCount = 0;
   code = 0;
   this->houseCode = houseCode; 
   memset(durations,0,sizeof(int)*BITS_IN_MESSAGE);
   pulseCount = 0;
   this->registerMessageHandler(this);
}

int ArduinoLightSensor::deviceType(void) {
   return DEVICE_ID;
};

char* ArduinoLightSensor::deviceName(void) {
   return (char*) DEVICE_NAME;
};

INTERRUPT_SAFE void ArduinoLightSensor::processPulse(long duration) {
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

void ArduinoLightSensor::decodeMessage(Message* message) {
   char* bytes = (char*) &message->code;
   int lightValue  = std::min(bytes[2] * 256 + bytes[1], 1000);
   message->type = 1;
   sprintf(message->text, "%ld, %x - light: %d",
           message->timestamp,
           message->code,
           lightValue);
};

int ArduinoLightSensor::numMessages(void) {
  return 1;
}

void ArduinoLightSensor::publishTopic(int messageNum, Message* message, char* buffer, int maxLength) {
   char* bytes = (char*) &message->code;
   snprintf(&buffer[strlen(buffer)], maxLength - strlen(buffer), "/%x", bytes[3]);
}

void ArduinoLightSensor::getMessageText(int messageNum, Message* message, char* buffer, int maxLength) {
   char* bytes = (char*) &message->code;
   int lightValue  = std::min(bytes[2] * 256 + bytes[1], 1000);
   sprintf(buffer, "%ld, %x - light: %d",
           message->timestamp,
           message->code,
           lightValue);
}

// to calculate the checksum first reverse the nibbles in each byte
// including the checksum 
// then add the 3 message bytes together and add 0x77 to get the 
// expected checksum which is in byte 0 
INTERRUPT_SAFE bool ArduinoLightSensor::validateChecksum(int code) {
   char* bytes = (char*) & code;
   char calcChecksum = 0x55;
   char checksum = ((bytes[0] >> 4) & 0x0F) + ((bytes[0] & 0x0F) << 4);

   for (int i = 1;i<4;i++) {
      calcChecksum += ((bytes[i] >> 4) & 0x0F) + ((bytes[i] & 0x0F) << 4);
   }

   if (calcChecksum == checksum) {
      return true;
   }

   return false;
}

