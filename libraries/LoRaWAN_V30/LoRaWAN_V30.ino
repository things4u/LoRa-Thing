/****************************************************************************************
* File:     LoRaWAN_V30.ino
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         20-11-2015
* Supported Hardware: ID150119-02 Nexus board with RFM95
* 
* Description
* 
* Minimal Uplink for LoRaWAN
* 
* This code demonstrates a LoRaWAN connection on a Nexus board. This code sends a messege every minute
* on channel 0 (868.1 MHz) Spreading factor 7.
* On every message the frame counter is raised
* 
* This code does not include
* Receiving packets and handeling
* Channel switching
* MAC control messages
* Over the Air joining* 
*
* Firmware version: 1.0
* First version
* 
* Firmware version 2.0
* Working with own AES routine
* 
* Firmware version 3.0
* Listening to receive slot 2 SF9 125 KHz Bw
* Created seperate file for LoRaWAN functions
****************************************************************************************/

/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/
#include <SPI.h>
#include "AES-128_V10.h"
#include "Encrypt_V30.h"
#include "LoRaWAN_V30.h"
#include "RFM95_V20.h"
#include "LoRaMAC_V10.h"
#include "Waitloop_V10.h"

/*
*****************************************************************************************
* GLOBAL VARIABLES
*****************************************************************************************
*/
// { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
unsigned char NwkSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

unsigned char AppSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

unsigned char DevAddr[4] = {
  0x02, 0x02, 0x04, 0x01
};

void setup() 
{
   //Initialize the UART
  Serial.begin(115200);

  Serial.println("setup: Init SPI port");
   //Initialise the SPI port
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000,MSBFIRST,SPI_MODE0));
  
  //Initialize I/O pins
  pinMode(DS2401,OUTPUT);
  pinMode(MFP,INPUT);
  pinMode(DIO0,INPUT);
  pinMode(DIO1,INPUT); 
  pinMode(DIO5,INPUT);
  pinMode(DIO2,INPUT);
  pinMode(CS,OUTPUT);
  pinMode(LED,OUTPUT);

  digitalWrite(DS2401,HIGH);
  digitalWrite(CS,HIGH);

  WaitLoop_Init();
  
  //Wait until RFM module is started
  WaitLoop(20); 
  Serial.println("setup: RFM module started");
  digitalWrite(LED,HIGH);  
}

void loop() 
{
  unsigned char Test = 0x00;

  unsigned char Sleep_Sec = 0x00;
  unsigned char Sleep_Time = 0x01;

  unsigned char Data_Tx[256];
  unsigned char Data_Rx[64];
  unsigned char Data_Length_Tx;
  unsigned char Data_Length_Rx = 0x00;

  //Initialize RFM module
  RFM_Init();

  while(1)
  {
    //Construct data
    memcpy(Data_Tx, "Hello Things4U 1", 14);
    Data_Length_Tx = 0x0E;

	Serial.print("loop: sending message ... ");
    Data_Length_Rx = LORA_Cycle(Data_Tx, Data_Rx, Data_Length_Tx);
	Serial.println("done");
	
    if(Data_Length_Rx != 0x00)
    {
      Test = Data_Rx[0];
	  Serial.println("loop: Data received");
    }

    //Delay of 10 s
    delay(60000);
  }//While(1)
}
