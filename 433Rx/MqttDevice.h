// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _MQTT_DEVICE
#define _MQTT_DEVICE

#include "Device.h"
#include "MqttBridge.h"

class MqttDevice : public Device, public MqttBridge {
  protected:

  public:
    MqttDevice(PubSubClient* client, char* topic);

    // optional if special topic handling is required
    virtual void publishTopic(int messageNum, Message* message, char* buffer, int maxLength);
    virtual void publishTopic(Message* message, char* buffer, int maxLength);
 };

#endif
