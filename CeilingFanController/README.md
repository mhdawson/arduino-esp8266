# CeilingFanController

This device provides an interface for controlling Hampton Bay
ceiling fans through mqtt.  Since I could not find a transitter/receiver
at the right freqency, it is built from a purchased remote
with the esp "pushing" the buttons on the remote by using
transistors to drive the buttons high or low.  The same is true for the device
select (0000 to 1111).  The esp can select the device by driving the
transistors connected to the select lines.

You can then control the ceiling fans by voice, remote on your phone or
scheduler When used with projects like:

* [AlexaMqttBridge](https://github.com/mhdawson/AlexaMqttBridge)
* [micro-app-remote-control](https://github.com/mhdawson/micro-app-remote-control)
* [micro-app-schedule-controller](https://github.com/mhdawson/micro-app-schedule-controller)

My favorite is:

```
"Alexa ask michael to turn on fan hi"
```

# Configuration

The following constants can be adjusted to fit your build
and configuration:

* Basic as part of ino file:
  * FAN_TOPIC - the topic on which the device listens
    for messages.  Messages are in the form of XXXX,command
    where XXXX is one of 0000 through 1111 to correspond to
    the code selected for the fan, and command is one of
    high, med, low or off.
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

The following is the schematic for the sensor hardware that I used:

![schematic](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/fan-circuit.png)

The jumper is to allow the nodeMCU to be powered from the 12v supply when the
USB connection is NOT in place.  You should not have the jumper closed and the
USB connected at the same time.  You can have the 12v supply connected and the
USB connection when the jumper is not in place so that you can update/test the
controller.

# Pictures

The following are a few pictures of my build.  The first shows the esp8266 circuit I built to control the buttons on the original remote.  The second that circuit wired into the original remote control (buttons and device select).  The last 2 shows the final packaging.

![picture1](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/fancontroller1.jpg)
![picture2](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/fancontroller2.jpg)
![picture3](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/fancontroller3.jpg)
![picture4](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/fancontroller4.jpg)

# Main Components

* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)
* [Remote Control](http://www.ebay.com/itm/232182237267?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT)  ~10 US on eBay.:
  ![remote control](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/fanremote.jpg)
