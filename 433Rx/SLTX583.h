// Copyright 2014-2018 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _SLTX583_DEVICE
#define _SLTX583_DEVICE

#include "Device.h"
#include "MqttDevice.h"

#define SLTX583_BITS_IN_MESSAGE 36

class SLTX583 : public Device, public MqttDevice {
   public:
      SLTX583(PubSubClient* client, char* topic);

      virtual int deviceType(void);
      virtual char* deviceName(void);

      virtual void processPulse(long duration);
      virtual void decodeMessage(Message* message);

      virtual int numMessages(void);
      virtual void getMessageText(int messageNum, Message* message, char* buffer, int maxLength);
      virtual void publishTopic(int messageNum, Message* message, char* buffer, int maxLength);

   private:
      bool syncFound;
      unsigned int bitCount;
      unsigned long code;
      long syncCount;
      unsigned int durations[SLTX583_BITS_IN_MESSAGE];
      long pulseCount;
      long repeatCount;
      long lastMessage;
};

#endif
