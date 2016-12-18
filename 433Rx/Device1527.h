// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _DEVICE1527_DEVICE
#define _DEVICE1527_DEVICE

#include "Device.h"
#include "MqttDevice.h"

#define BITS_IN_MESSAGE_1527 24

class Device1527 : public Device, public MqttDevice {
   public:
      Device1527(int pulseWidth, int pulseSlack, int minRepeats,
                 PubSubClient* client, char* topic);

      virtual int deviceType(void);
      virtual char* deviceName(void);

      virtual void processPulse(long duration);
      virtual void decodeMessage(Message* message);
      virtual void publishTopic(int messageNum, Message* message, char* buffer, int maxLength);
      
   private:
      int _pulseWidth;
      int _pulseSlack;
      int _minRepeats;
      bool syncFound;
      unsigned int bitCount;
      unsigned char receivedCode[BITS_IN_MESSAGE_1527 + 1];
      unsigned char lastMessage[BITS_IN_MESSAGE_1527];
      unsigned int durations[BITS_IN_MESSAGE_1527];
      long pulseCount;
      long repeatCount;
};

#endif
