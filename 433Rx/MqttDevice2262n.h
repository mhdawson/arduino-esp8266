// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _MQTT_DEVICE2262N_DEVICE
#define _MQTT_DEVICE2262N_DEVICE

#include "MqttDevice.h"
#include "Device2262n.h"

class MqttDevice2262n : public Device2262n, public MqttDevice {
  public:
    MqttDevice2262n(int pulseWidth, int pulseSlack, int minRepeats, PubSubClient* client, char* topic);

  private:
  
};

#endif
