# LoRa-Thing

This directory contains a series of programs that can be used
to make a LoRa sensor. The directory contains sketches for types of sensors, as well
as libraries that support these sketches with common functions.

Documentation is found on: http://things4u.github.io

- LORAWAN_3.1-htu: Sensor to report temperature and humidity. 
    The node is based on simple LoRa node (one frequency only) originally developed by
	hoperf.nl and modified/worked on to support various sensors.

- esp-lmic-v1.5: a working version of a sensor built on ESP8266 that connects to 
    thethingsnetwork
- esp-lmic-v1.5-ds18b20: A version of ESP8266/RFM95 sensor device reporting temperature 
	values of DS18B20 temperature sensor (using waterproof DALLAS sensor)
