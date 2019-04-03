# LightAndTemp Sensor

This sensor reports the current temperature and light intensity.

# Configuration

The following constants can be adjusted to fit your build
and configuration.

* The wifi configuration and mqttServer address are configured in
  WirelessConfig.h which must be added to the project:
  * ssid - The id of the wireless network to connect to.
  * pass - the password for the wireless network.
  * mqttServerString - the mqtt server to which to connect to in
    TCP/IP dot notation (ex "10.1.1.186").
  * mqttServerPort - port for the mqtt server to connect to 

If using certificates the follwing are also required
  * client_cert - bytes in DER format
  * client_cert_len - length in bytes of client cert
  * client_key -  bytes in DER format
  * client_key_len - length in bytes of key

* Device configuration from configuration for device in SensorConfig.h
  * LIGHT_TOPIC - mqtt topic to post light intensity (0-1024).
  * TEMP_TOPIC - mqtt topic to post temperature to.
  * TEMP_TOPIC - mqtt topic to topic to listen on for LED on/off message

See the sample [WirelessConfig.h.sample](https://github.com/mhdawson/arduino-esp8266/blob/master/TempAndLightSensor/WirelessConfig.h.sample) more more details;

# Building

To build you need to add the following:

* [OneWire](https://github.com/PaulStoffregen/OneWire) (must be recent version
  that supports esp8266).
* [DallasTemperture](http://milesburton.com/Main_Page?title=Dallas_Temperature_Control_Library)
* [Arduino ESP library](https://github.com/esp8266/Arduino)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

libraries as libraries in your Arduino IDE

Once these are installed, you can then add your SensorConfig.h and
WirelessConfig.h files and the compile and flash your esp8266.

# Schematic

The following is the schematic for the sensor hardware that I
used:

![schematic](https://github.com/mhdawson/arduino-esp8266/blob/master/TempAndLightSensor/LightAndTempSensor.png)

# Pictures

The following are a few pictures of my build:

![picture1](https://github.com/mhdawson/arduino-esp8266/blob/master/TempAndLightSensor/TempAndLightSensorCase.jpg)

# Main Components

* [DS18B20 Temp Sensor](https://www.ebay.ca/itm/DALLAS-18B20-DS18B20-TO-92-Wire-Digital-Thermometer-Temperature-IC-Sensor/152757780361)
* [NodeMCU D1](http://www.ebay.com/itm/NodeMCU-Lua-ESP-12-WeMos-D1-Mini-WIFI-4M-Bytes-Development-Board-Module-ESP8266-/321989574625)
* [Light Sensor - 5516 GL5516 NT00183](http://www.ebay.ca/itm/20Pcs-Light-Sensitive-Inductor-Photo-Resistor-Photoresistor-5516-GL5516-NT00183-/261420488934?hash=item3cdde018e6)
