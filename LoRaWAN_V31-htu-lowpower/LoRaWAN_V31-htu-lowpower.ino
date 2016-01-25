/****************************************************************************************
* File:     LoRaWAN_V31-htu.ino
* Author:   Maarten Westenberg
* Company:  Things4U
* 
* E-mail:   mw12554@hotmail.com
*
* Publishing temperature and humidity to thethingsnetwork
* This is the low-power version, for use on battery power.
* So receiving functions are disabled...
* 
* Based on the example program made by Gerben of Ideetron http://www.ideetron.nl/LoRa
* Changed and extended to support HTU21d temperature/humidity sensors
* running on an Arduino Pro-Mini
****************************************************************************************/

/****************************************************************************************
* Created on:         20-01-2015
* Supported Hardware: Arduino Pro-Mini board with RFM95 and HTU21d sensor
* 
* Description
* 
* Minimal Uplink for LoRaWAN
* 
* This code demonstrates a LoRaWAN connection on an Arduino Nano board. 
* This code sends a JSON message every minute on channel 0 (868.1 MHz) Spreading factor 7.
* On every message the frame counter is raised
* 
* This code does not include
* Receiving packets and handeling (although the original receiver code is there in comments)
* Channel switching
* MAC control messages
* Over the Air joining* 
*
* Jan 2016; Raised the firmware Version level to 3.1
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
#include "LowPower.h"					// For the Arduino only (not the RFM95)
#include "Wire.h"
/*
*****************************************************************************************
* GLOBAL VARIABLES
*****************************************************************************************
*/

// Sensor
#include <HTU21D.h>
HTU21D myHumidity;						// Init Sensor(s)
  
// This key is for thethingsnetwork
unsigned char NwkSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

// If we want, we can add our own key here
unsigned char AppSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

// Please change address below to be specific for your nodes
//
unsigned char DevAddr[4] = {
  0x01, 0x01, 0x01, 0x01
};

// --------------------------------------------------------------------
// SETUP FUNCTION
// --------------------------------------------------------------------
void setup() 
{
 
  //Initialize the UART Serial communication
  Serial.begin(115200);

  //Initialise the SPI port
  Serial.println("setup: Init SPI port");

  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000,MSBFIRST,SPI_MODE0));
  
  //Initialize I/O pins. Some are not used, need to clean up.
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

//---------------------------------------------------------------------
// MAIN LOOP
//---------------------------------------------------------------------
void loop() 
{
  unsigned int  Frame_Counter = 0x0000;
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
	//return;										// In normal operation, return and do not send the 998 values.
  }
  
  // Floats are not defined in Arduino printf
  // Therefore make some sort of split between integer and fractional part using type casting.
  
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
  //unsigned char Data_Rx[64];				// No receiver supported
  unsigned char Data_Length_Tx;
  //unsigned char Data_Length_Rx = 0x00;	// No receiver supported

  //Initialize RFM module
  RFM_Init();

  //Construct data
  memcpy(Data_Tx, msg, strlen(msg));
  Data_Length_Tx = strlen(msg);

  Serial.print("loop: sending message: "); Serial.println(msg);
  
  //Data_Length_Rx = LORA_Cycle(Data_Tx, Data_Rx, Data_Length_Tx);	// Send and receive
  LORA_Send_Data(Data_Tx, Data_Length_Tx, Frame_Counter);			// Send only! (for battery sensors)

  Frame_Counter++;

  Serial.println("Sleep 90 seconds"); Serial.println();
  delay(200);										// Delay if Sleep to succeed if all comms is done
  RFM_Write(0x01,0x00);								// Put RFM95 in Sleep mode
  
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);	// Put Arduino in Sleep mode 12 times 8 seconds
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);   // Either this or a for-loop (whatever takes less memory)
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
