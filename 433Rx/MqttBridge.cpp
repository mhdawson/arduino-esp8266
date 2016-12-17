// Copyright 2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "MqttBridge.h"
#include "MqttDevice.h"

#define MAX_TOPIC_LENGTH 1024
#define MAX_MESSAGE_LENGTH 256

MqttBridge::MqttBridge(PubSubClient* client, char* topic) {
  _client = client;
  _topic = topic;
}


void MqttBridge::handleMessage(Message* message) {
  char topicBuffer[MAX_TOPIC_LENGTH];
  char messageBuffer[MAX_MESSAGE_LENGTH];

  Device* messageDevice = ((Device*) message->device);
  for (int i=0;i<messageDevice->numMessages();i++) {
    topicBuffer[0] = 0;
    messageBuffer[0] = 0;
    strncpy(topicBuffer, _topic, MAX_TOPIC_LENGTH);
    ((MqttDevice*) messageDevice)->publishTopic(i, message, topicBuffer, MAX_TOPIC_LENGTH); 
    messageDevice->getMessageText(i, message, messageBuffer, MAX_MESSAGE_LENGTH); 
    if (strlen(topicBuffer) != 0) {
      _client->publish(topicBuffer, messageBuffer);
    }
  }
}
