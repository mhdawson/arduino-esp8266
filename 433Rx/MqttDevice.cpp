// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "Device.h"
#include "MqttDevice.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#define MAX_TOPIC_LENGTH 1024
#define MAX_MESSAGE_LENGTH 256


MqttDevice::MqttDevice(PubSubClient* client, char* topic) {
  _client = client;
  _topic = topic;
}


void MqttDevice::handleNextMessage(Message* message) {
  char topicBuffer[MAX_TOPIC_LENGTH];
  char messageBuffer[MAX_MESSAGE_LENGTH];

  Device* messageDevice = ((Device*) message->device);
  for (int i=0;i<messageDevice->numMessages();i++) {
    topicBuffer[0] = 0;
    messageBuffer[0] = 0;
    strncpy(topicBuffer, _topic, MAX_TOPIC_LENGTH);
    ((MqttDevice*) message->device)->publishTopic(i, message, topicBuffer, MAX_TOPIC_LENGTH); 
    messageDevice->getMessageText(i, message, messageBuffer, MAX_MESSAGE_LENGTH); 
    if (strlen(topicBuffer) != 0) {
      _client->publish(topicBuffer, messageBuffer);
    }
  }
}


void MqttDevice::publishTopic(Message* message, char* buffer, int maxLength) {
}


void MqttDevice::publishTopic(int messageNum, Message* message, char* buffer, int maxLength) {
  publishTopic(message, buffer, maxLength);
}
