/*******************************************************************************
 * Copyright (c) 2016 Maarten Westenberg
 * based on work of Thomas Telkamp, Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This sketch sends a valid LoRaWAN packet with payload a DS18B 20 temperature 
 * sensor reading that will be processed by The Things Network server.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1, 
*  0.1% in g2). 
 *
 * Change DEVADDR to a unique address! 
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Do not forget to define the radio type correctly in config.h, default is:
 *   #define CFG_sx1272_radio 1
 * for SX1272 and RFM92, but change to:
 *   #define CFG_sx1276_radio 1
 * for SX1276 and RFM95.
 *
 * History: 
 * Jan 2016, Modified by Maarten to run on ESP8266. Running on Wemos D1-mini
 *******************************************************************************/
 
// Use ESP declarations. This sketch does not use WiFi stack of ESP
//#include <ESP8266WiFi.h>
#include <ESP.h>
#include <Base64.h>

// All specific changes needed for ESP8266 need be made in hal.cpp if possible
// Include ESP environment definitions in lmic.h (lmic/limic.h) if needed
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//---------------------------------------------------------
// LoRaWAN settings (for thethingsnetwork)
//---------------------------------------------------------

// LoRaWAN Application identifier (AppEUI)
// Not used in this example
static const u1_t APPEUI[8]  = { 0x02, 0x00, 0x00, 0x00, 0x00, 0xEE, 0xFF, 0xC0 };

// LoRaWAN DevEUI, unique device ID (LSBF)
// Not used in this example
static const u1_t DEVEUI[8]  = { 0x42, 0x42, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };

// LoRaWAN NwkSKey, network session key 
// Use this key for The Things Network
static const u1_t DEVKEY[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// LoRaWAN AppSKey, application session key
// Use this key to get your data decrypted by The Things Network
static const u1_t ARTKEY[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
static const u4_t DEVADDR = 0x01010101 ; // <-- Change this address for every node! ESP8266 node 0x01

//---------------------------------------------------------
// Sensor declarations
//---------------------------------------------------------
#define ONE_WIRE_BUS 2					// GPIO2/D4
#define S_DALLAS 1						// Make 1 to use Dallas sensor(s)

#if S_DALLAS==1
#include "OneWireESP.h"					// This is an ESP8266 specific library!
										// The standard ESP library is edited to support
#include "DallasTemperature.h"
  OneWire oneWire(ONE_WIRE_BUS);
  // Pass our oneWire reference to Dallas Temperature. 
  DallasTemperature sensors(&oneWire);
  int numberOfDevices; 					// Number of temperature devices found
#endif

//---------------------------------------------------------
// APPLICATION CALLBACKS
//---------------------------------------------------------

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}

int debug=1;
uint8_t mydata[64];
static osjob_t sendjob;

// Pin mapping
// XXX We have to see whether all these pins are really used
// if not, we can use them for real sensor work.
lmic_pinmap pins = {
  .nss = 15,			// Make D8/GPIO15, is nSS on ESP8266
  .rxtx = 2, 			// D4/GPIO2. For placeholder only,
						// Do not connected on RFM92/RFM95
  .rst = 0,  			// Make D0/GPIO16, Needed on RFM92/RFM95? (probably not)
  .dio = {5, 4, 3},		// Specify pin numbers for DIO0, 1, 2
						// D1/GPIO5,D2/GPIO4,D3/GPIO3 are usable pins on ESP8266
						// NOTE: D3 not really usable when UART not connected
						// As it is used during bootup and will probably not boot.
						// Leave D3 Pin unconnected for sensor to work
};

void onEvent (ev_t ev) {
    //debug_event(ev);

    switch(ev) {
      // scheduled data sent (optionally data received)
      // note: this includes the receive window!
      case EV_TXCOMPLETE:
          // use this event to keep track of actual transmissions
          Serial.print("Event EV_TXCOMPLETE, time: ");
          Serial.println(millis() / 1000);
          if(LMIC.dataLen) { // data received in rx slot after tx
              //debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
              Serial.println("Data Received!");
          }
          break;
       default:
          break;
    }
}

// ----------------------------------------------------
// This function prepares a message for the LoRaWAN network
// The message will be sent multiple times.
//
void do_send(osjob_t* j){
	  Serial.println();
      Serial.print("Time: "); Serial.println(millis() / 1000);
      // Show TX channel (channel numbers are local to LMIC)
      Serial.print("Send, txCnhl: "); Serial.println(LMIC.txChnl);
      Serial.print("Opmode check: ");
      // Check if there is not a current TX/RX job running
    if (LMIC.opmode & (1 << 7)) {
      Serial.println("OP_TXRXPEND, not sending");
    } else {
      Serial.print("ok, ready to send: ");
	  Serial.print((char *)mydata);
	  Serial.println();
	  
#if S_DALLAS==1
	  // Dallas sensors (can be more than 1) have channel codes 3 and above!
	  uint8_t ind;
	  DeviceAddress tempDeviceAddress; 			// We'll use this variable to store a found device address
	  sensors.requestTemperatures();
	  for(int i=0; i<numberOfDevices; i++)
	  {
		// Search the wire for address
		if(sensors.getAddress(tempDeviceAddress, i)) {
			float tempC = sensors.getTempC(tempDeviceAddress);
			// Output the device ID
			if (debug>=1) {
				Serial.print(F("! DS18B20 dev ("));
				Serial.print(i);
			}
			int ival = (int) tempC;					// Make integer part
			int fval = (int) ((tempC - ival)*10);	// Fraction. Has same sign as integer part
			if (fval<0) fval = -fval;				// So if it is negative make fraction positive again.
			sprintf((char *)mydata,"{\"t\":\"%d.%d\"}",ival,fval);
			if (debug>=1) {
				Serial.print(") ");
				Serial.println((char *)mydata);
			}
	  } 
	 //else ghost device! Check your power requirements and cabling
	}
#else
	strcpy((char *) mydata,"Hello ESP8266\n");
#endif	  
	  
	  
      // Prepare upstream data transmission at the next possible time.
      LMIC_setTxData2(1, mydata, strlen((char *)mydata), 0);
	  //LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
    }
    // Schedule a timed job to run at the given timestamp (absolute system time)
    os_setTimedCallback(j, os_getTime()+sec2osticks(120), do_send);
         
}

// ----------------------------------------------------
// Remove the Serial messages once the unit is running reliable
// 
void setup() {
  Serial.begin(115200);
  Serial.println("Starting");
  
#if S_DALLAS==1
	sensors.begin();
	numberOfDevices = sensors.getDeviceCount();
	if (debug>=1) {
		Serial.print("DALLAS #:");
		Serial.print(numberOfDevices); 
		Serial.println(" ");
	}
#endif

  // LMIC init
  os_init();
  Serial.println("os_init() finished");
  
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  Serial.println("LMIC_reet() finished");
  
  // Set static session parameters. Instead of dynamically establishing a session 
  // by joining the network, precomputed session parameters are be provided.
  LMIC_setSession (0x1, DEVADDR, (uint8_t*)DEVKEY, (uint8_t*)ARTKEY);
  Serial.println("LMIC_setSession() finished");
  
  // Disable data rate adaptation
  LMIC_setAdrMode(0);
  Serial.println("LMICsetAddrMode() finished");
  
  // Disable link check validation
  LMIC_setLinkCheckMode(0);
  // Disable beacon tracking
  LMIC_disableTracking ();
  // Stop listening for downstream data (periodical reception)
  LMIC_stopPingable();
  // Set data rate and transmit power (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7,14);
  //
  Serial.println("Init done");
}


//---------------------------------------------------------
// main loop
// Loop is simple: read sensor value and send it to the LoRaWAN
// network.
//---------------------------------------------------------

void loop() {
Serial.println("loop: Starting");
strcpy((char *) mydata,"Starting ESP8266 Dallas\n");

// The do_send function puts a message in the queue and then puts
// itself to sleep. When waking up, will again work on queue again.

do_send(&sendjob);				// Put job in run queue(send mydata buffer)
delay(10);

while(1) {
  
  os_runloop_once();			// Let the server run its jobs
  
  delay(100);					// Wait a second. Note: actual wait time is longer
								// to comply with LoRaWAN transmission time standards.
  }
}


