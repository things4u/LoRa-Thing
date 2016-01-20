LMiC 1.5 port to ESP8266
========================
(c) M. Westenberg, jan 2016 

This library demonstrates that LMIC 1.5 works on ESP8266 after making the necessary modifications. 
LMIC is the original LoRaMAC-in-C library which was developed by IBM, and was modified 
by Matthijs Kooijman and Thomas Telkamp to run on Arduino (the Teensy etc.)

After some modifications it runs on the ESP8266 as well.

Supported MCU's
---------------
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
You can find the documentation at http://things4u.github.io

