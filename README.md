
# MQTT Broker Check
This is based on the Basic MQTT example from the PubSubClient library.
It has been modified to use Username and Password credentials for the remote broker and the NTPClient library is added as an indicator of end-to-end connectivity.
Use in conjunction with MQTT Explorer (https://mqtt-explorer.com/) to test connectivity and message flow.  

 It uses the WiFi library to connect to a WiFi network and the PubSubClient library to connect to the MQTT server.  
 It uses the NTPClient library to get the current time from an NTP server.  
 It also:
  - prints the MAC address of the ESP32 to the terminal.
  - prints the allocated IP address of the ESP32 to the terminal.
  - appends the MAC address to the MQTT client ID prefix and prints the new ID to the terminal (to identify the device in MQTT Explorer).
  - publishes the current time to the MQTT server every (1) seconds (with time zone offset).
  - subscribes to the topic "inTopic" and prints any messages received to the terminal.

> NOTE
> 1) The MQTT server used in this example was a private broker via a local AP.
> 2) Update for your own broker details or use test.mosquitto.org (and modify as no credentials required).
> 3) The MQTT server must be running and accessible from the ESP32 device.
> 4) This code was created to aid checking connectivity, firewall rules etc in troubleshooting. A **known-good** MQTT client (as of April 2025).
> 5) If I'm understanding correctly, the standard example will only subscribe upon reconnecting to the broker, not if successfully connected first time.
> 6) (Re)connection to the MQTT broker is still blocking, FYI.  
