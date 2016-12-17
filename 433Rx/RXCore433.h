// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _RX_CORE_433
#define _RX_CORE_433

#include "Device.h"

class RXCore433 {
  public:
    RXCore433(int interrupt);
    void handleMessage();
    bool registerDevice(Device* newDevice);

  private:
    Message _lastHandledMessage;
    ICACHE_RAM_ATTR static void handleInterrupt();
};

#endif
