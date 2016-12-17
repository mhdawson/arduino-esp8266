// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _MQTT_DEVICE
#define _MQTT_DEVICE

#include "DeviceMessageHandler.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class MqttDevice : public DeviceMessageHandler {
  public:
    MqttDevice(PubSubClient* client, char* topic);

    virtual void handleNextMessage(Message* message);

    // optional override if special topic handling is required
    virtual void publishTopic(int messageNum, Message* message, char* buffer, int maxLength);
    virtual void publishTopic(Message* message, char* buffer, int maxLength);

  private:
    PubSubClient* _client;
    char* _topic;
 };

#endif
