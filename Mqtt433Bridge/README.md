# Mqtt433Bridge

This device provides a bridge between 433MHz devices
and mqtt topics.  It can be used as a lower cost version of
[PI433WirelessRecvManager](https://github.com/mhdawson/PI433WirelessRecvManager).  Since it uses a $5 NodeMCU D1 mini instead
of a Raspberry Pi, it costs ~ $10 to build, as opposed
to $50-75 for the Raspberry build.

Due to limitations related to interrupt handlers having
to be in memory, it can only support about 9 different
devices at the same time. Otherwise, it supports the same
devices as the PI433WirelessRecvManager project, and is a port/refactor/improved version
of the code from that probject. At some point
I'll likely push the changes back and unify
the code so that it can build for the
esp, arduino and Raspberry Pi using the same codebase.

# Configuration

The wifi configuration and mqttServer address are configured in
WirelessConfig.h which must be added to the project:

  * ssid - The id of the wireless network to connect to.
  * pass - the password for the wireless network.
  * mqttServer - the mqtt server to which to connect to.

As an example see the sameple file: [WirelessConfig.h](https://github.com/mhdawson/arduino-esp8266/blob/master/WirelessConfig.h).

The mqtt topics used for each device are configured in the ".ino" file for the project as mentioned in the section titled "Building".

# Building

To build you need to add the following:

* [Arduino ESP library](https://github.com/esp8266/Arduino)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

as libraries in your Arduino IDE.

Due to to the fact that interrupt handles have to be in memory, not all of the devices can be supported at the same time.

You will need to copy the device "".h" and .cpp" files from the [433Rx](https://github.com/mhdawson/arduino-esp8266/tree/master/433Rx)
project.  Only copy the files for the devices you are going to enable.
In the ".ino" file,  you will then have to comment out the include for the ".h" file for each of the devices not being enable, as well as commenting out receiver.registerDevice(...) line for the same devices.

Once these are installed, you can then add your and
Wifi configuration file as described above and then compile and flash your esp8266.

You likely also want to adjust the receiver.registerDevice(...) lines for each of the enabled devices to reflect the mqtt topics to which the device data will be posted.

## Currently supported devices
- Generic 2262 based device (motion detector, door sensor, etc.)
- Generic 1527 based device (door sensor, etc.)
- Lacross 141 temperature sensor
- Blueline Power monitor
- Meat thermometer
- Custom devices from [arduino-sensors](https://github.com/mhdawson/arduino-sensors)

### Lacross 141
The Lacross 141 is available at Canadian tire and often goes on sale for $10-$15
[cnd tire link](http://www.canadiantire.ca/en/pdp/la-crosse-weather-station-with-colour-frame-1427129p.html#.VV6MmlKznt8)
![Lacross 141](https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/Lacross-package.jpg?raw=yes)

### Generic 2262 Devices

Many different devices should work.  Currently this project only supports one protocol and I believe
there are a few different ones.  I'll look to add support for the others when I come across them.  These
are a few that I've used succesfully.  

#### Motion detector

Device: - 2262n - parameters(350,50,4,&lt;your topic&gt;)
Available from ebay, select 433 for frequency and 2262 for chipset
[ebay link motion detector](http://www.ebay.ca/itm/Wireless-Standard-PIR-Motion-Detector-Sensor-315-433-Mhz-1-5-3-3-4-7-M-/171089657359?var=&hash=item0)

![Motion Detector](https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/433Motion.jpg)

#### Door Sensor
Device: - 2262n - parameters(350,50,4,&lt;your topic&gt;)
Available from ebay, select 433 for frequency and 2262 for chipset
[ebay link door sensor](http://www.ebay.ca/itm/Wireless-Door-Window-Entry-Detector-Sensor-Contact-315-433-Mhz-/181183039531?var=&hash=item0)
![Door sensor]https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/433Door.jpg)

#### Smoke Detector
Device: - 2262n - parameters(350,50,4,&lt;your topic&gt;)
Available from ebay.
[ebay link smoke detector](http://www.ebay.ca/itm/321225011653?_trksid=p2057872.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT)

![Smoke Detector](https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/433Smoke.jpg)

#### Remote controls for 120V switch
Device: - 2262n - parameters(200,75,2,&lt;your topic&gt;)
Available from eay.
[ebay link 120v switch](http://www.ebay.ca/itm/381117176383?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT)

![433Switch](https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/433switch.jpg)

### Generic 1527 Device

Many different devices should work.  It is similar to 2262 but has less redundancy and more bits
available in the message.  They are not configured by jumpers.  

This is one that I have used so far:

#### Door Sensor
Device: - 1527 - parameters(350,50,4,&lt;your topic&gt;)
Available from ebay  
[ebay link door sensor](http://www.ebay.ca/itm/311256729170?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT)
![Door sensor](https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/1527Door.jpg)

### Meat thermometer

Generic meat thermometer.  

![meat thermometer](https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/MeatThermometer1.jpg)

I have it working with one that looks like this from Amazon, and have ordered a couple from ebay to validate that all of the ones that look like these work the same. [ebay link](http://www.ebay.com/itm/122090166085?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT)

## Adding a new device

I plan on adding a "how to" with respect to figuring out the encoding and adding a new device in this project [PI433WirelessRecvManager-Decoding](https://github.com/mhdawson/PI433WirelessRecvManager-Decoding).

## Reciver

The receiver I used is an RXB6:

![picture receiver 1](https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/receiver_1.jpg?raw=yes)
![picture receiver 2](https://raw.githubusercontent.com/mhdawson/PI433WirelessRecvManager/master/pictures/receiver_2.jpg?raw=yes)

It works well from the basement to the second floor in my house and is available on ebay for about $2 Canadian: [RXB6](http://www.ebay.ca/itm/Super-heterodyne-OOK-Wireless-Receiver-Module-Strong-Interference-433MHZ-116dBm-/271638472090?pt=LH_DefaultDomain_0&hash=item3f3eea259a).  While this one works well for me, any other 433 receiver should work with this project as well.

# Schematic

The following is the schematic for the bridge hardware that I used:

![schematic](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-433MqttBridge.jpg)

The diode is a IN4148 and is there to clamp the output from the receiver down to 3.3v to match the esp input.

# Pictures
The following are a few pictures of my build:

![picture1](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-433-bridge-1.jpg)

![picture2](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-433-bridge-2.jpg)

# Main Components

* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)
* [RXB6](http://www.ebay.com/itm/1pcs-RXB6-433Mhz-Superheterodyne-Wireless-Receiver-Module-for-Arduino-ARM-AVR-/401085388270?hash=item5d628d55ee:g:90UAAOSwr7ZW4BQZ)
