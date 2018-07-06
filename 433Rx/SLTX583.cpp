// Copyright 2014-2018 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "SLTX583.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEVICE_ID 13
#define DEVICE_NAME "SLTX583"

#define MIN_PULSE_LENGTH 300
#define SYNC_DURATION_MIN 7000
#define SYNC_DURATION_MAX 12000
#define ONE_PULSE_MIN_LENGTH 3200
#define MIN_REPEAT_COUNT 2

SLTX583::SLTX583(PubSubClient* client, char* topic) : Device(), MqttDevice(client, topic) {
   syncCount = 0;
   syncFound = false;
   bitCount = 0;
   code = 0;
   memset(durations,0,sizeof(int)*SLTX583_BITS_IN_MESSAGE);
   pulseCount = 0;
   repeatCount = 0;
   lastMessage = 0;
   this->registerMessageHandler(this);
}

int SLTX583::deviceType(void) {
   return DEVICE_ID;
};

char* SLTX583::deviceName(void) {
   return (char*) DEVICE_NAME;
};

INTERRUPT_SAFE void SLTX583::processPulse(long duration) {
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
 
      // ok we have all bits that we expect 
      if (SLTX583_BITS_IN_MESSAGE == bitCount) {
         // decode the first 4 bits to validate that we have a match with this
         // device. They should always be 0101 = 5
         for (int i = 0; i < 4; i++) {
             code = code << 1;
             if (ONE_PULSE_MIN_LENGTH < durations[i]) {
                code = code | 0x1;
             }
         }

         if (5 == code) { 
            code = 0;
            for (int i = 4; i < SLTX583_BITS_IN_MESSAGE; i++) {
               code = code << 1;
               if (ONE_PULSE_MIN_LENGTH < durations[i]) {
                  code = code | 0x1;
               }
            }

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
                     newMessage->timestamp = time(NULL); 
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
         } 

        // ok wait for the next message
        syncFound = false;
      } 
   } else {
      if ((duration > SYNC_DURATION_MIN) && (duration < SYNC_DURATION_MAX)) {
         code = 0;
         syncFound = true;
         syncCount = 0;
         bitCount = 0;
         pulseCount = 0;
      } else {
         syncCount = 0;
      }
   }
};

void SLTX583::decodeMessage(Message* message) {
   char* bytes = (char*) &message->code;
   message->type = 1;
   char channel = ((bytes[2] & 0x30) >> 4) + 1;
   char humidity = bytes[0];
   float temp = (bytes[2] & 0xF)*256 + bytes[1];
   snprintf(message->text, MAX_MESSAGE_TEXT_LENGTH, "%ld, %x - temp: %.1f, hum: %d, chan: %d", message->timestamp, message->code, temp/10, humidity, channel);
};

int SLTX583::numMessages(void) {
  return 2;
}

void SLTX583::publishTopic(int messageNum, Message* message, char* buffer, int maxLength) {
   char* bytes = (char*) &message->code;
   char channel = ((bytes[2] & 0x30) >>4) + 1;
   char id = bytes[3];
   if (messageNum == 0) {
    
      snprintf(&buffer[strlen(buffer)], maxLength - strlen(buffer), "/%x/%x/temp", channel, id);
   } else {
      snprintf(&buffer[strlen(buffer)], maxLength - strlen(buffer), "/%x/%x/humidity", channel, id);
   }
}

void SLTX583::getMessageText(int messageNum, Message* message, char* buffer, int maxLength) {
   char* bytes = (char*) &message->code;

   if (messageNum == 0) {
      float temp = (bytes[2] & 0xF)*256 + bytes[1];
      snprintf(buffer,
               maxLength,
               "%ld, %x - temp: %.1f", message->timestamp, message->code, temp/10);
   } else {
      char humidity = bytes[0];
      snprintf(buffer,
               maxLength,
               "%ld, %x - humidity: %d", message->timestamp, message->code, humidity);
   }
}
