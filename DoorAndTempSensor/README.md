# DoorAndTemp Sensor

This sensor combines the Door open/close sensor along with a temperature
sensor.  I used it to monitor the state of our garage door and
the temperature in the garage.  The mqtt topics and ids may seem a bit odd but
that is because I was replacing as existing sensor build as detailed
[here](https://github.com/mhdawson/arduino-sensors/tree/master/DoorAndTempSensor).

# Configuration

The following constants can be adjusted to fit your build
and configuration.

* Basic as part of ino file:
  * TRANSMIT_INTERVAL_SECONDS - The interval at which the sensor sends
    updates.  In addition the sensor will send an update when the door
    open/close state changes.
  * OPEN_CLOSE_PIN - the digital pin on which the door sensor is wired.
  * DS18B20_PIN - the digital pin on which the temp sensor is wired.


* Wifi configuration, from WirelessConfig.h:
  * ssid - The id of the wireless network to connect to.
  * pass - the password for the wireless network


* Device configuration from configuration for device (ex ..\devices-config\GarageDoorConfig.h):
  * DEVICE_2272_ID - The 2272 code used for this sensor. If you have
    multiple sensors make this unique for each one.
  * DOOR_TOPIC - mqtt topic to most door open/close messages.
  * TEMP_TOPIC - mqtt topic to post temperature messages.

# Building

To build you need to add the following:

* [OneWire](https://github.com/PaulStoffregen/OneWire) (must be recent version
  that supports esp8266).
*  [DallasTemperture](http://milesburton.com/Main_Page?title=Dallas_Temperature_Control_Library)
* [Arduino ESP library](https://github.com/esp8266/Arduino)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

libraries as libraries in your Arduino IDE

Once these are installed, you can then add your device and
Wifi configuration files and the compile and flash your esp8266.

# Schematic

The following is the schematic for the sensor hardware that I
used:

![schematic](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-door-diag.jpg)

# Pictures

The following are a few pictures of my build:

![picture1](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-door-pict1.jpg)
![picture2](https://raw.githubusercontent.com/mhdawson/arduino-esp8266/master/pictures/esp-door-pict2.jpg)

# Main Components

* [DS18B20 Temp Sensor](http://www.ebay.ca/itm/10PCS-Waterproof-Digital-Thermal-Probe-or-Sensor-DS18B20-/130702483183?hash=item1e6e799eef)
* [Door contact](http://www.ebay.ca/itm/5-Set-Recessed-Door-Window-Contact-Magnetic-Reed-Switch-Sensor-Security-Alarm-/381198534569?hash=item58c13407a9:g:U7IAAOSw-7RVCm9F)
* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)

# TODO

The mqtt server is currently hard code in this line:

```
IPAddress server(10, 1, 1, 186);
```

need to separate this configuration out into the Wifi config.:
