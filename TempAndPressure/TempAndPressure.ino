#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SI7021.h>

// device specifics
#include "WirelessConfig.h"

#define TRANSMIT_INTERVAL_SECONDS 120

IPAddress server(mqttServer[0], mqttServer[1],
                 mqttServer[2], mqttServer[3]);

Adafruit_BMP280 bmp; 
SI7021 sensor;

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;
PubSubClient client(wclient, server);

void setup() {
  // wait a bit to let things settle
  delay(1000);

  Serial.begin(115200);
  Serial.println("starting");
  
  // use pins 4,5 for SDA, SCL
  Wire.begin(D4, D5);
  sensor.begin(D4, D5);

  // start the bmp sensor
  if (!bmp.begin(0x76)) {  
    Serial.println("No BMP280 sensor");
  }

  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);
}

void loop() {
  char conversionBuffer[20];
  
  client.loop();

  // make sure we are good for wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
  
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      return;
    }
    Serial.println("WiFi connected");
  }

  if (!client.connected()) {
    if (client.connect("sensclient")) {
      client.subscribe("inTopic");
    }
  }

  client.publish("house/mcuSensor/1/temp1", 
                 dtostrf(bmp.readTemperature(), 2, 2, conversionBuffer));
  
  client.publish("house/mcuSensor/1/temp2", 
                 dtostrf(((float)sensor.getCelsiusHundredths())/100, 2, 2, conversionBuffer));

  client.publish("house/mcuSensor/1/pressure", 
                 dtostrf(bmp.readPressure(), 2, 2, conversionBuffer));

  sprintf(conversionBuffer, "%d", sensor.getHumidityPercent());
  client.publish("house/mcuSensor/1/humidity", conversionBuffer);
                 
  delay(TRANSMIT_INTERVAL_SECONDS * 1000);

}
