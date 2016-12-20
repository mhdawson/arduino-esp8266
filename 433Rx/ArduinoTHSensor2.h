// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

// Version of the TH sensor will more precision in temperature

#ifndef _ARDUINO_TH_SENSOR2_DEVICE
#define _ARDUINO_TH_SENSOR2_DEVICE

#include <stdint.h>
#include "Device.h"
#include "MqttDevice.h"

#define BITS_IN_TH_MESSAGE 40
#define NUM_BYTES BITS_IN_TH_MESSAGE/8

class ArduinoTHSensor2 : public Device, public MqttDevice {
   public:
      ArduinoTHSensor2(PubSubClient* client, char* topic);

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
      unsigned long thecode;
      unsigned char code[NUM_BYTES];
      int codeByte;
      long syncCount;
      unsigned int durations[BITS_IN_TH_MESSAGE];
      long pulseCount;
      bool validateChecksum(char* bytes);

      float getTemperature(char* bytes);
      int getHumidity(char* bytes);
      int getDeviceId(char* bytes);
};

#endif
