// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _BLUELINE_DEVICE
#define _BLUELINE_DEVICE

#include "Device.h"

#define BITS_IN_MESSAGE 32

class BluelineDevice : public Device {
   public:
      BluelineDevice(long houseCode);

      virtual int deviceType(void);
      virtual char* deviceName(void);

      virtual void processPulse(long duration);
      virtual void decodeMessage(Message* message);
      virtual void publishTopic(Message* message, char* buffer, int maxLength);

   private:
      bool syncFound;
      unsigned int bitCount;
      unsigned int code;
      unsigned int durations[BITS_IN_MESSAGE];
      long houseCode;
      long pulseCount;
};

#endif
