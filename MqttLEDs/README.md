# MqttLEDs

This project allows you to control a 2812B LED strip of lights
through MQTT.  It current supports the following commands:

* off - turn all lights off
* range - set a range of lights to a specific color
* clear+range - turn existing lights off and then set a range of lights 
  a specific color
* fade - fade from the current color to another color over a period of time
* cycle - cycle between 2 or more colors over a period of time
* fire - simimulate the flickering of a fire

The formats for each command as as described in the sections which
follow. Each each case the components of the command are separated
by spaces and you send the command to the topic configured for
the device as descibed in the `building` section.

## off

`off`

cancels all existing queued commands.

## range

`range start end R G B`

where:

* start - number between 0 and x -1 where x is the number of lights
  in the strip
* end - number between 0 and x -1 where x is the number of lights
  in the strip
* R - number between 0 and 255 for red component of the color to bet set
* G - number between 0 and 255 for green component of the color to bet set
* B - number between 0 and 255 for blue component of the color to bet set

cancels all existing queued commands.

## clear+range

Same as for range except the command at the start is `clear+range` instead
of `range`.

cancels all existing queued commands.

## fade

`fade start end R G B duration`

where:

* start - number between 0 and x -1 where x is the number of lights
  in the strip
* end - number between 0 and x -1 where x is the number of lights
  in the strip
* R - number between 0 and 255 for red component of the color to bet set
* G - number between 0 and 255 for green component of the color to bet set
* B - number between 0 and 255 for blue component of the color to bet set
* duration - number of milliseconds over which to change the color.

Fade commands are queued.  If you send multiple fade commads they will
be executed one after another. For example, if you submit

```
fade 1 10 255 255 255 5000
fade 1 10 0   0   0   5000
fade 1 10 255 255 255 5000
```

then the lights will fade to full on over 5 seconds, then they will fade to
off over five seconds and then they will fad to full on over 5 seconds.

## cycle

`cycle count duration start end R1 G1 B1 .... Rn Gn Bn`

where:

* count - the number of different colors to cycle between
* duration - the number of millieconds taken to fade from one color
  to another
* start - number between 0 and x -1 where x is the number of lights
  in the strip
* end - number between 0 and x -1 where x is the number of lights
  in the strip
* R1 - number between 0 and 255 for red component of the first color in the cycle
* G1 - number between 0 and 255 for green component of the first color in the cycle
* B1 - number between 0 and 255 for blue component of the first color in the cycle
* Rn - number between 0 and 255 for red component of the last color in the cycle
* Gn - number between 0 and 255 for green component of the last color in the cycle
* Bn - number between 0 and 255 for blue component of the last color in the cycle

There must count R G B tripples. For example to cycle between 3 colors:

`cycle 3 5000 1 10 255 0 0 0 255 0 255 0 0`

where `255 0 0 0 255 0 255 0 0` are the 3 different colors to cycle between
with the fade between each color being 5000 milliseconds (5 seconds).  In this
example the lights will continuously fade between these 3 colors until
another command is executed.


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


# Pictures


# Main Components

* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)
