# Mqtt433Bridge

This device provides an a bridge between 433MHz devices
and mqtt.

# Configuration




# Building

To build you need to add the following:

* [Arduino ESP library](https://github.com/esp8266/Arduino)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

libraries as libraries in your Arduino IDE.

Once these are installed, you can then add your device and
Wifi configuration files and the compile and flash your esp8266.


# Schematic

The following is the schematic for the bridge hardware that I used:

![schematic](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-433MqttBridge.jpg)

# Pictures

The following are a few pictures of my build:

![picture1](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-433-bridge-1.jpg)

![picture2](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-433-bridge-2.jpg)

# Main Components

* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)
* [RXB6](http://www.ebay.com/itm/1pcs-RXB6-433Mhz-Superheterodyne-Wireless-Receiver-Module-for-Arduino-ARM-AVR-/401085388270?hash=item5d628d55ee:g:90UAAOSwr7ZW4BQZ)
