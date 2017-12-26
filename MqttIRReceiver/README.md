# MqttIRReciver 

This device provides a simple IR receiver.  It does not 
know about any specific codes, instead the raw timing
information is sent by the receive  through mqtt to a back end.
It is up to the back end to decode the IR signals.
This approach allows the device itself to be quite simple,
while being able to receive from a large range of devices with
proper back-end control.

The back end control will initially be managed through
[micro-app-ir-to-mqtt-bridge](https://github.com/mhdawson/micro-app-ir-to-mqtt-bridge).

The receiver collects raw IR data until a `space` larger than
END_TIME_VALUE (0.1s at time of writing) and which point it sends
all of the values in a single MQTT message.  The message is
formatted as `A1, A2, .... An`. Each of the Ax is the number
microseconds in HEX for the `space` or `mark`.  The message starts with a
`space` which is followed by a `mark`, then by a `space` and so on
alternating between the two. For example:

```
20faf,7b,1e25,160,257,5a2,15c,43d,527,1f7
```

# Configuration

The wifi configuration and mqttServer address are configured in
WirelessConfig.h which must be added to the project:

  * ssid - The id of the wireless network to connect to.
  * pass - the password for the wireless network.
  * mqttServerString - the mqtt server to which to connect to in
    TCP/IP dot notation (ex "10.1.1.186").
  * mqttServerPort - port for the mqtt server to connect to 

As an example see the sameple file:
[WirelessConfig.h](https://github.com/mhdawson/arduino-esp8266/blob/master/WirelessConfig.h).

The mqtt topics used for each device are configured in the ".ino"
ile for the project as mentioned in the section titled "Building".

# Building

To build you need to add the following:

* [Arduino ESP library](https://github.com/esp8266/Arduino)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

as libraries in your Arduino IDE.

You will then want to modify `#define IR_TOPIC "house/ir1"` to specify the
topic that the the device will publish to. Note that you should have each
device publish to a different topic as the IR_TOPIC is also used to set the
mqtt ID for the device.

# Schematic

![schematic](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/IR-receiver-circuit.jpg)

# Pictures

![build1](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/IR-receiver-1.jpg)
![build2](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/IR-receiver-2.jpg)

# Case 

You can 3D print the case that I designed.  The stl file is:

![3D model](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/IR%20Case.stl)

In order to assemble the overall device I used krazy glue to glue in the
IR receiver and a small amount of electrical tape rolled up at the end of the WeMos D1
(on the opposite side of the USB port) to help wedge in the WeMos.

# Main Components

* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)
* [TSOP 38238](https://www.vishay.com/docs/82491/tsop382.pdf)
