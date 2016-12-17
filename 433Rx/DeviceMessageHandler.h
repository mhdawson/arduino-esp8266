// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _DEVICE_MESSAGE_HANDLER
#define _DEVICE_MESSAGE_HANDLER

#include "MessageQueue.h"

class DeviceMessageHandler {
  public:
    DeviceMessageHandler() {};

    virtual void handleNextMessage(Message* message)= 0;

  private:
};

#endif
