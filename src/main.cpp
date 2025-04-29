/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is based on the Basic MQTT example from the PubSubClient library.
It has been modified to use Username and Password credentials for the remote broker and the NTPClient library is added as an indicator of end-to-end connectivity.
Use in conjunction with MQTT Explorer (https://mqtt-explorer.com/) to test connectivity and message flow.

 It uses the WiFi library to connect to a WiFi network and the PubSubClient library to connect to the MQTT server.
 It uses the NTPClient library to get the current time from an NTP server.
 It also:
  - prints the MAC address of the ESP32 to the terminal
  - prints the allocated IP address of the ESP32 to the terminal
  - appends the MAC address to the MQTT client ID prefix and prints the new ID to the terminal (to identify the device in MQTT Explorer)
  - publishes the current time to the MQTT server every (1) seconds (with time zone offset)
  - subscribes to the topic "inTopic" and prints any messages received to the terminal

NOTE
1) The MQTT server used in this example was a private broker via a local AP.
2) Update for your own broker details or use test.mosquitto.org (and modify as no credentials required).
3) The MQTT server must be running and accessible from the ESP32 device.
4) This code was created to aid checking connectivity, firewall rules etc in troubleshooting. A **known-good** MQTT client (as of April 2025).
5) If I'm understanding correctly, the standard example will only subscribe upon reconnecting to the broker, not if successfully connected first time.
6) (Re)connection to the MQTT broker is still blocking, FYI.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Basic MQTT example
This sketch demonstrates the basic capabilities of the library.
It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
  it receives. NB - it assumes the received payloads are strings not binary

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define MSG_BUFFER_SIZE (50)

const char *ssid = "CHANGE ME"; // your network SSID (name)
const char *password = "CHANGE ME"; // your network password
const char *mqtt_server = "CHANGE ME"; // mqtt server address
const char *mqtt_user = "CHANGE ME"; // your MQTT username
const char *mqtt_password = "CHANGE ME"; // your MQTT password
String clientId = "myDevice_";

unsigned long previousMillis = 0;
unsigned long lastMsg = 0;
unsigned long now;
unsigned int interval = 1000; // interval between messages
unsigned long startAttemptTime = millis();
char msg[MSG_BUFFER_SIZE];
bool sub = true; // flag to indicate if we need to subscribe

WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

///////////////////////////////////////////////////////////////////////////////
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("\nMessage arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
///////////////////////////////////////////////////////////////////////////////
void reconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password))
    {
      Serial.println("MQTT connected!");
      //  Once connected, publish an announcement...
      mqttClient.publish("outTopic", "hello world");
      // ... and resubscribe
      // mqttClient.subscribe("inTopic"); no longer needed here as we do this in loop()
      sub = true; // Set sub flag to true to subscribe to topics after reconnecting
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
///////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(57600);
  Serial.print("\n\n/// BUILD: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.print(__TIME__);
  Serial.println(" ///\n");

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress()); // Retrieve the MAC address of the ESP32

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqtt_callback);

  WiFi.begin((char *)ssid, (char *)password);
  Serial.print("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
  {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Failed to connect to WiFi. Restarting...");
    ESP.restart();
  }
  else
  {
    Serial.println("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  timeClient.setTimeOffset(3600); // GMT +1 = 3600
  timeClient.begin();             // initialise Network Time Protocol client
  Serial.println("NTP Client started");

  /* Append MAC to client ID */
  clientId += WiFi.macAddress();
  Serial.println("MQTT client ID: " + clientId);

  sub = true; // Set resub flag to true to subscribe to topics for first-time connection
}
///////////////////////////////////////////////////////////////////////////////
void loop()
{
  if (!mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password))
  {
    reconnect();
  }
  /* Subscribe to topics - just not every loop through */
  if (sub)
  {
    mqttClient.subscribe("inTopic");
    sub = false;
  }
  mqttClient.loop();

  now = millis();
  /* Check if it's time to publish the next message */
  if (now - lastMsg > interval)
  {
    lastMsg = now;
    Serial.print(".");
    if (!timeClient.update())
    {
      timeClient.forceUpdate();
    }
    timeClient.getFormattedTime().toCharArray(msg, MSG_BUFFER_SIZE);
    mqttClient.publish(clientId.c_str(), msg);
    mqttClient.publish((clientId + "/IPaddr").c_str(), WiFi.localIP().toString().c_str()); // Publish the IP address to the MQTT server
  }
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////