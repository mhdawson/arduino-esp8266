
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "WirelessConfig.h"
#include "ESP8266WiFiGeneric.h"

#define TRANSMIT_INTERVAL_SECONDS 120

IPAddress server(10, 1, 1, 186);

// 2272 codes used to signify open or closed door
#define OPEN_CLOSE_PIN D5
#define NUM_CHARS_IN_MESSAGE 12
#define DEVICE_2272_ID "0101FFFF000"
#define CLOSED "1"
#define OPEN "0"

// device id used in message transmitted for the
// temp sensor
#define MIN_TEMP_INTERVAL 10
#define MAX_TEMP_MESSAGE_SIZE 100
#define DS18B20_SENSOR_ID 0x24
#define DS18B20_PIN D4  // don't use D0 or D2 as can interfere with boot
OneWire ds(DS18B20_PIN);
DallasTemperature tempSensors(&ds);

void callback(const MQTT::Publish& pub) {
  Serial.println(pub.payload_string());
  // handle message arrived
}

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;
PubSubClient client(wclient, server);

int counter = 0;
int lastState = 0;

void setup() {
  delay(1000);
  // Setup console
  pinMode(OPEN_CLOSE_PIN, INPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();

  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);

  client.set_callback(callback);

  // first reading always seems to be wrong, read it early and
  // throw it away
  tempSensors.requestTemperatures();
}

void loop() {
  client.loop();
  
  // we transmit the state of the door and the temperature every
  // TRANSMIT_INTERVAL_SECONDS or when the state (open/close) of
  // the door changes
  delay(100);
  counter++;
  int state = digitalRead(OPEN_CLOSE_PIN);
  if ((lastState != state) || (counter == (TRANSMIT_INTERVAL_SECONDS * 10))) {
    Serial.println("checking");
    char message[NUM_CHARS_IN_MESSAGE+1];
    memset(message, 0, NUM_CHARS_IN_MESSAGE+1);
    strncat(message, DEVICE_2272_ID, NUM_CHARS_IN_MESSAGE);
    if(state == 0) {
      strncat(message, OPEN, NUM_CHARS_IN_MESSAGE);
    } else {
      strncat(message, CLOSED, NUM_CHARS_IN_MESSAGE);
    }
     
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connecting to ");
      Serial.print(ssid);
      Serial.println("...");
      WiFi.begin(ssid, pass);
  
      if (WiFi.waitForConnectResult() != WL_CONNECTED)
        return;
      Serial.println("WiFi connected");
    }
  
    if (!client.connected()) {
      if (client.connect("arduinoClient")) {
        client.subscribe("inTopic");
      }
    }

    // send out door state
    client.publish("outTopic", message);

    // send out temperature
    if (counter > (MIN_TEMP_INTERVAL *10)) {
      // don't send out temperature too often as we'll get 
      // incorrect values if we sample too often
      char tempMessage[MAX_TEMP_MESSAGE_SIZE];
      char floatBuffer[10];
      tempSensors.requestTemperatures();
      float currentTemp = tempSensors.getTempCByIndex(0);
      snprintf(tempMessage, MAX_TEMP_MESSAGE_SIZE, "0, 0 - temp %s",
               dtostrf(currentTemp, 4, 2, floatBuffer));
      client.publish("outTopic", tempMessage);
    }

    counter = 0;
    lastState = state;
  }
}
