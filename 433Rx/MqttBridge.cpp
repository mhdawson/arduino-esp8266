// Copyright 2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "MqttBridge.h"

MqttBridge::MqttBridge(PubSubClient* client, char* topic) {
  _client = client;
  _topic = topic;
}

void MqttBridge::handleMessage(Message* message) {
  _client->publish(_topic, message->text);
}
