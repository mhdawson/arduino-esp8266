
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESP8266WiFiGeneric.h"

// device specifics
#include "WirelessConfig.h"
#include "GarageSensorConfig.h"

#define TRANSMIT_INTERVAL_SECONDS 120

#define LOOP_DELAY 100

IPAddress server(mqttServer[0], mqttServer[1],
                 mqttServer[2], mqttServer[3]);

// 2272 codes used to signify open or closed door
#define OPEN_CLOSE_PIN D5
#define NUM_CHARS_IN_MESSAGE 12

#define CLOSED "0"
#define OPEN "1"

#define MAX_TOPIC_LEN 128
#define MIN_TEMP_INTERVAL 10
#define MAX_TEMP_MESSAGE_SIZE 100

#define DS18B20_PIN D4  // don't use D0 or D2 as can interfere with boot
OneWire ds(DS18B20_PIN);
DallasTemperature tempSensors(&ds);

#define BUTTON_PUSH_TIME 2000
#define GARAGE_REMOTE_BUTTON D7
#define GARAGE_DOOR_TOPIC "house/garage"

WiFiClient wclient;
ESP8266WiFiGenericClass wifi;

int remainingButtonTime = 0;
void callback(char* topic, uint8_t* message, unsigned int length) {
  // the only message we current expect is to push the
  // button for the garage door (open/close) which is "DOOR"
  std::string messageBuffer((const char*) message, length);
  Serial.println(messageBuffer.c_str());
  if (strcmp(messageBuffer.c_str(), "DOOR") == 0) {
    remainingButtonTime = BUTTON_PUSH_TIME;
    digitalWrite(GARAGE_REMOTE_BUTTON, HIGH);
  }
}

PubSubClient client(mqttServerString, mqttServerPort, callback, wclient);

int counter = 0;
int lastState = 0;

void setup() {
  delay(1000);

  // Setup pins
  pinMode(OPEN_CLOSE_PIN, INPUT);
  pinMode(GARAGE_REMOTE_BUTTON, OUTPUT);
  digitalWrite(GARAGE_REMOTE_BUTTON, LOW);

  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Started");

  // turn of the Access Point as we are not using it
  wifi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  // first reading always seems to be wrong, read it early and
  // throw it away
  tempSensors.requestTemperatures();
}

void loop() {
  client.loop();
  // we transmit the state of the door and the temperature every
  // TRANSMIT_INTERVAL_SECONDS or when the state (open/close) of
  // the door changes
  delay(LOOP_DELAY);

  // make sure we are good for wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.reconnect();

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Failed to reconnect WIFI");
      Serial.println(WiFi.waitForConnectResult());
      delay(100);
      return;
    }
  }

    
  if (!client.connected()) {
    if (client.connect("arduinoClient2")) {
      Serial.println("PubSub connected");
      client.subscribe(GARAGE_DOOR_TOPIC);
    } else {
      Serial.println("PubSub failed to connect");
    }
  }

  // check if is time for the remote button to be turned off
  if (remainingButtonTime > 0 ) {
    remainingButtonTime = remainingButtonTime - LOOP_DELAY;
    if (remainingButtonTime <= 0) {
      remainingButtonTime = 0;
      digitalWrite(GARAGE_REMOTE_BUTTON, LOW);
    }
  }

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
     
    // send out door state
    char doorTopic[MAX_TOPIC_LEN];
    memset(doorTopic, 0, MAX_TOPIC_LEN);
    strncat(doorTopic, DOOR_TOPIC, MAX_TOPIC_LEN);
    strncat(doorTopic, "/", MAX_TOPIC_LEN);
    strncat(doorTopic, message, MAX_TOPIC_LEN);
    client.publish(doorTopic, message);

    // send out temperature
    if (counter > (MIN_TEMP_INTERVAL *10)) {
      // don't send out temperature too often as we'll get 
      // incorrect values if we sample too often
      char tempMessage[MAX_TEMP_MESSAGE_SIZE];
      char floatBuffer[10];
      tempSensors.requestTemperatures();
      float currentTemp = tempSensors.getTempCByIndex(0);
      snprintf(tempMessage, MAX_TEMP_MESSAGE_SIZE, "0, 0 - temp: %s",
               dtostrf(currentTemp, 4, 2, floatBuffer));
      client.publish(TEMP_TOPIC, tempMessage);
    }

    counter = 0;
    lastState = state;
  }
}
