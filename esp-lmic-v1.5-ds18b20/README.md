LMiC 1.5 port to ESP8266 with Temperature Sensor
================================================
(c) M. Westenberg, jan 2016 

This sketch implement an temperature sensor for LoRa network.
It contains the full LoRa stack on an ESP8266 (based on LMIC-1.5 port) with a
temperature sensor (DS18B20 Dallas/OnWire).

In this sketch sensor is connected to pin D4

The sketch NEEDS the lmic-2.5 libraries in the libraries directory. 
Make sure you download these to your system as well.

Supported ESP8266 MCU's
------------------------
In principle the Library and the example sketch will work on every ESP8266, as until 
today all of my sketches work on NodeMCU 0.9 and NodeMCU 1.0 systems and lookalikes.
I do however have less favourable experience with bare ESP-xx implementations.

The sensor has been tested on my Wemos D1-mini MCU, but more to follow.

Notes
-----
At least in the first version no WiFi functions are switched on. Handling the WiFi
stack takes the MCU a great deal of time and may result in Watchdog resets, mostly
because the user applications run too long and the MCU cannot handle the WiFi stack 
anymore.

Solution is to keep function as short as possible, avoid unnecessary Serial communication,
do not loop in the loop() section, and insert some delay() statements to give way
to the system processing tasks.

Documentation
-------------
Some material has been put together to help build this node. 
You can find the documentation at http://things4u.github.io in hardware section

