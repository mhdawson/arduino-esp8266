// Copyright 2014-2016 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "MqttDevice2262n.h"

MqttDevice2262n::MqttDevice2262n(int pulseWidth, int pulseSlack, int minRepeats, PubSubClient* client, char* topic) : 
  Device2262n(pulseWidth, pulseSlack, minRepeats), MqttDevice(client, topic) {
  this->registerMessageHandler(this);
};

