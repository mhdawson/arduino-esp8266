// Copyright 2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _MQTTBRIDGE_DEVICE
#define _MQTTBRIDGE_DEVICE

#include "Device2262n.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class MqttBridge : public DeviceMessageHandler {
   public:
      MqttBridge(PubSubClient* client, char* topic);
      virtual void handleMessage(Message* message);

   private:
      PubSubClient* _client;
      char* _topic;
};

#endif
