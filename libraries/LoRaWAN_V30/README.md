# Nexus-LoRaWAN-Mote
With this code the Nexus will send messages that will be accepted by the LoRaWAN network. 
Also the Nexus board will listen in receiveslot to if the gateway is sending back a message.

Transmit
- Channel 0: 868.100 MHz
- Datarate 5 = Spreading factor 7, 125 kHz BW
- Full power

Receive
- Channel: 869.525 MHz
- Datarate 3: Spreading factor 9, 125 kHz BW

The timing for the receive slot is handeld in LoRaMAC_V10.cpp

This code will not:
- Change channels
- Change datarates
- Send MAC commands

# Hardware
The Nexus board is fitted with an ATMEGA328P and a RFM95 module

To adapt this code to other hardware change the LoRaWAN_V30.h

From the DIO's only 0,1 and 5 are used
