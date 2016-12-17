// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "MqttDevice.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#define MAX_TOPIC_LENGTH 1024
#define MAX_MESSAGE_LENGTH 256


MqttDevice::MqttDevice(PubSubClient* client, char* topic) : MqttBridge(client, topic) {
  this->registerMessageHandler(this);
}


void MqttDevice::publishTopic(Message* message, char* buffer, int maxLength) {
}


void MqttDevice::publishTopic(int messageNum, Message* message, char* buffer, int maxLength) {
  publishTopic(message, buffer, maxLength);
}
