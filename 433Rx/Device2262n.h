// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _DEVICE2262N_DEVICE
#define _DEVICE2262N_DEVICE

#include "Device.h"

#define BITS_IN_MESSAGE_2262 24
#define TRISTATE_MESSAGE_LENGTH 12

class Device2262n : public Device {
  public:
    Device2262n(int pulseWidth, int pulseSlack, int minRepeats);

    virtual int deviceType(void);
    virtual char* deviceName(void);
    virtual void processPulse(long duration);
    virtual void decodeMessage(Message* message);

  private:
    int _pulseWidth;
    int _pulseSlack;
    int _minRepeats;
    bool syncFound;
    unsigned int bitCount;
    unsigned char tristateCode[TRISTATE_MESSAGE_LENGTH];
    unsigned char lastMessage[TRISTATE_MESSAGE_LENGTH];
    unsigned int durations[BITS_IN_MESSAGE_2262];
    long pulseCount;
    long repeatCount;
};

#endif
