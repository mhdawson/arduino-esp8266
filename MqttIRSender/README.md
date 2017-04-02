# MqttIRSender

This device provides a simple IR sender.  It does not 
know about any specific codes, instead the raw timing
information is sent through mqtt and the device simply
sends out codes based on those timings. This approach
allows the device itself to be quite simple, while
being able to control a large range of devices with
proper back-end control.

The back end control will initially be mangaed through
[UniversalIR](https://github.com/mhdawson/UniversalIR).

# Configuration

The wifi configuration and mqttServer address are configured in
WirelessConfig.h which must be added to the project:

  * ssid - The id of the wireless network to connect to.
  * pass - the password for the wireless network.
  * mqttServerString - the mqtt server to which to connect to in
    TCP/IP dot notation (ex "10.1.1.186").
  * mqttServerPort - port for the mqtt server to connect to 

As an example see the sameple file: [WirelessConfig.h](https://github.com/mhdawson/arduino-esp8266/blob/master/WirelessConfig.h).

The mqtt topics used for each device are configured in the ".ino" file for the project as mentioned in the section titled "Building".

# Building

To build you need to add the following:

* [Arduino ESP library](https://github.com/esp8266/Arduino)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

as libraries in your Arduino IDE.

You will then want to modify `#define IR_TOPIC "house/ir"` to specify the 
topic that the the device should listen on.

# Schematic

![schematic](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/IRSender-circuit.png)

# Pictures

![build1](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/IRSender.jpg)

# Main Components

* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)
