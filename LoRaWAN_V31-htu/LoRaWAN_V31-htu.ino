/****************************************************************************************
* File:     LoRaWAN_V31-htu.ino
* Author:   Maarten Westenberg
* Company: Things4U
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   mw12554@hotmail.com
*
* Publishing temperature and humidity to thethingsnetwork
* 
* Based on the example program made by Gerben of Ideetron
* Changed and extended to support HTU21d temperature/humidity sensors
* running on an Arduino Pro-Mini
****************************************************************************************/

/****************************************************************************************
* Created on:         20-01-2015
* Supported Hardware: Arduino Pro-Mini board with RFM95
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
#include "LowPower.h"

#include "Wire.h"
/*
*****************************************************************************************
* GLOBAL VARIABLES
*****************************************************************************************
*/

// Sensor
#include <HTU21D.h>
  HTU21D myHumidity;			// Init Sensor(s)
  
// This key is for thethingsnetwork
unsigned char NwkSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

// If we want, we can add our own key here
unsigned char AppSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

// Things4U has Device Addresses 0x02 0x02 0x04 0x??
//
unsigned char DevAddr[4] = {
  0x01, 0x01, 0x01, 0x01
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
  
  myHumidity.begin();
  myHumidity.readHumidity();			// First read value after starting does not make sense.
  Serial.println("HTU21D started");
}

void loop() 
{
  char msg[64];
  // HTU21 or SHT21
  float humd = myHumidity.readHumidity();
  delay(100);
  float temp = myHumidity.readTemperature();
  if (((int)temp == 999) || ((int)temp == 998)) {
	Serial.println(F("HTU temp timeout"));
  }
  if (((int)humd != 999) && ((int)humd !=998 )) {	// Timeout (no sensor) or CRC error
	Serial.print("Humidity: ");
	Serial.println(humd);
	Serial.print("Temperature: ");
	Serial.println(temp);
  }
  else {
	Serial.println("Error reading temperature and humidity");
  }
  
  // Floats are not defined in Arduino printf
  int it = (int) temp;						// Make integer part
  int ft = (int) ((temp - it)*10);			// Fraction. Has same sign as integer part
  if (ft<0) ft = -ft;						// So if it is negative make fraction positive again.
  int ih = (int) humd;						// Make integer part
  int fh = (int) ((humd - ih)*10);			// Fraction. Has same sign as integer part
  if (fh<0) fh = -fh;						// So if it is negative make fraction positive again.
  
  sprintf(msg,"{\"t\":\"%d.%d\",\"h\":\"%d.%d\"}",it,ft,ih,fh);
  
  unsigned char Test = 0x00;

  unsigned char Sleep_Sec = 0x00;
  unsigned char Sleep_Time = 0x01;

  unsigned char Data_Tx[256];
  unsigned char Data_Rx[64];
  unsigned char Data_Length_Tx;
  unsigned char Data_Length_Rx = 0x00;

  //Initialize RFM module
  RFM_Init();

  //Construct data
  memcpy(Data_Tx, msg, strlen(msg));
  Data_Length_Tx = strlen(msg);

  Serial.print("loop: sending message: ");
  Serial.println(msg);
  
  Data_Length_Rx = LORA_Cycle(Data_Tx, Data_Rx, Data_Length_Tx);
	
  if(Data_Length_Rx != 0x00)
  {
      Test = Data_Rx[0];
	  Serial.println("loop: Data received");
  }

  //delay(90000);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
