# MessageWiatingInidicator

This device provides an indication of whether voicemail messages
are waiting or not (and can be used for any other indicator as well).
Our phone does not have a good indicator so we use this to make
it more obvious when messages are waiting.

# Configuration

The following constants can be adjusted to fit your build
and configuration:

* Basic as part of ino file:
  * BLINK_INTERVAL_SECONDS - The interval at which the led blinks.
  * LED_PIN - the digital pin on which the led is wired.
  * MESSAGE_WAITING_TOPIC - the topic on which the device listens
    for messages indicating the status of whether messages are
    waiting or not.

* Wifi configuration, from WirelessConfig.h:
  * ssid - The id of the wireless network to connect to.
  * pass - the password for the wireless network.
  * mqttServer - array with the IPV4 components for the mqtt
    server.

# Building

To build you need to add the following:

* [Arduino ESP library](https://github.com/esp8266/Arduino)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

libraries as libraries in your Arduino IDE.

Once these are installed, you can then add your device and
Wifi configuration files and the compile and flash your esp8266.

# Schematic

The following is the schematic for the sensor hardware that I
used:

![schematic](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/esp-message-waiting-diag.jpg)

# Pictures

The following are a few pictures of my build:

![picture1](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/messageWaiting1.jpg)
![picture2](https://github.com/mhdawson/arduino-esp8266/blob/master/pictures/messageWaiting2.jpg)

# Main Components

* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)

