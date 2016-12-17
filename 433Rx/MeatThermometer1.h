// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _MEAT_THERMOMETER1_DEVICE
#define _MEAT_THERMOMETER1_DEVICE

#include "Device.h"

#define BITS_IN_MESSAGE 32

class MeatThermometer1  : public Device {
  public:
    MeatThermometer1();

    virtual int deviceType(void);
    virtual char* deviceName(void);

    virtual void processPulse(long duration);
    virtual void decodeMessage(Message* message);
    virtual void publishTopic(Message* message, char* buffer, int maxLength);

  private:
    bool syncFound;
    unsigned int bitCount;
    unsigned long code;
    long currentBit;
    long syncCount;
    long shortCount;
};

#endif
