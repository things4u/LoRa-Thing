/******************************************************************************************
*
* File:        RFM95_V20.cpp
* Author:      Gerben den Hartog
* Compagny:    Ideetron B.V.
* Website:     http://www.ideetron.nl/LoRa
* E-mail:      info@ideetron.nl
******************************************************************************************/

/****************************************************************************************
*
* Created on:         23-10-2015
* Supported Hardware: ID150119-02 Nexus board with RFM95
*
* History:
*
* Firmware version: 1.0
* First version for LoRaWAN
****************************************************************************************/

#include <SPI.h>
#include "LoRaMAC_V10.h"
#include "RFM95_V20.h"
#include "LoRaWAN_V30.h"
#include "Arduino.h"
#include "Waitloop_V10.h"

/*
*****************************************************************************************
* Description: Function used to initialize the RFM module on startup
*****************************************************************************************
*/

void RFM_Init()
{
  //Switch RFM to sleep
  RFM_Write(0x01,0x00);

  //Set RFM in LoRa mode
  RFM_Write(0x01,0x80);

  //Set RFM in Standby mode wait on mode ready
  RFM_Write(0x01,0x81);
  while (digitalRead(DIO5) == LOW)
  {
  }

  //Set carrair frequency
  // 868.100 MHz / 61.035 Hz = 14222987 = 0xD9068B
  RFM_Write(0x06,0xD9);
  RFM_Write(0x07,0x06);
  RFM_Write(0x08,0x8B);

  //PA pin (maximal power)
  RFM_Write(0x09,0xFF);

  //BW = 125 kHz, Coding rate 4/5, Explicit header mode
  RFM_Write(0x1D,0x72);

  //Spreading factor 7, PayloadCRC On
  RFM_Write(0x1E,0x74);

  //Rx Timeout set to 37 symbols
  RFM_Write(0x1F,0x25);

  //Preamble length set to 8 symbols
  //0x0008 + 4 = 12
  RFM_Write(0x20,0x00);
  RFM_Write(0x21,0x08);

  //Low datarate optimization off AGC auto on
  RFM_Write(0x26,0x04);

  //Set LoRa sync word
  RFM_Write(0x39,0x34);

  //Set IQ to normal values
  RFM_Write(0x33,0x27);
  RFM_Write(0x3B,0x1D);

  //Set FIFO pointers
  //TX base adress
  RFM_Write(0x0E,0x80);
  //Rx base adress
  RFM_Write(0x0F,0x00);

}

/*
*****************************************************************************************
* Description : Function for sending a package with the RFM
*
* Arguments   : *RFM_Tx_Package Pointer to arry with data to be send
*               Package_Length  Length of the package to send
*****************************************************************************************
*/

void RFM_Send_Package(unsigned char *RFM_Tx_Package, unsigned char Package_Length)
{
  unsigned char i;
  unsigned char RFM_Tx_Location = 0x00;

  //Switch DIO0 to TxDone
  RFM_Write(0x40,0x40);

  //Set carrier frequency
  // 868.100 MHz / 61.035 Hz = 14222987 = 0xD9068B
  RFM_Write(0x06,0xD9);
  RFM_Write(0x07,0x06);
  RFM_Write(0x08,0x8B);

  //Switch to SF7 CRC on payload on
  RFM_Write(0x1E,0x74);

  //Set IQ to normal values
  RFM_Write(0x33,0x27);
  RFM_Write(0x3B,0x1D);

  //Set payload length to the right length
  RFM_Write(0x22,Package_Length);

  //Get location of Tx part of FiFo
  RFM_Tx_Location = RFM_Read(0x0E);

  //Set SPI pointer to start of Tx part in FiFo
  RFM_Write(0x0D,RFM_Tx_Location);

  //Write Payload to FiFo
  for (i = 0;i < Package_Length; i++)
  {
    RFM_Write(0x00,*RFM_Tx_Package);
    RFM_Tx_Package++;
  }

  //Switch RFM to Tx
  RFM_Write(0x01,0x83);

  //Wait for TxDone
  while(digitalRead(DIO0) == LOW)
  {
  }
}

/*
*****************************************************************************************
* Description  : Switches the RFM module to receive
*****************************************************************************************
*/

message_t RFM_Receive()
{
  message_t Message_Status = NO_MESSAGE;

  unsigned char RFM_Interrupt;
  unsigned char Timer = 0x00;

  //Set carrier freqeuncy
  //869.525 / 61.035 = 14246334 = 0xD961BE
  RFM_Write(0x06,0xD9);
  RFM_Write(0x07,0x61);
  RFM_Write(0x08,0xBE);

  //Change DIO 0 to RxDone
  RFM_Write(0x40,0x00);

  //Switch to SF 9 payload on CRC on
  RFM_Write(0x1E,0x94);

  //Invert IQ
  RFM_Write(0x33,0x67);
  RFM_Write(0x3B,0x19);

  //Switch RFM to Single reception
  RFM_Write(0x01,0x86);

  //Wait on mode ready
  while(digitalRead(DIO5) == LOW)
  {
  }

  //Wait until RxDone or Timeout
  //Wait until timeout or RxDone interrupt
  while((digitalRead(DIO0) == LOW) && (digitalRead(DIO1) == LOW))
  {
  }

  //Get interrupt register
  RFM_Interrupt = RFM_Read(0x12);

  //Check for Timeout
  if(digitalRead(DIO1) == HIGH)
  {
    Message_Status = TIMEOUT;
  }

  //Check for RxDone
  if(digitalRead(DIO0) == HIGH)
  {
    //Check CRC
    if((RFM_Interrupt & 0x20) != 0x20)
    {
      Message_Status = CRC_OK;
    }
    else
    {
      Message_Status = WRONG_MESSAGE;
    }
  }

  //Clear interrupt register
  RFM_Write(0x12,0xE0);

  //Switch rfm to standby
  RFM_Write(0x01,0x81);

  return Message_Status;
}

/*
*****************************************************************************************
* Description : This function will check for a CRC error and get the received data
          from the RFM.
*
* Arguments   : *RFM_Rxd_package  Pointer to arry for the received data
*
* Returns   : Lenght of package
*****************************************************************************************
*/

unsigned char RFM_Get_Package(unsigned char *RFM_Rx_Package)
{
  unsigned char i;
  unsigned char RFM_Package_Length        = 0x0000;
  unsigned char RFM_Package_Location      = 0x0000;
 
  RFM_Package_Location = RFM_Read(0x10); /*Read start position of received package*/
  RFM_Package_Length   = RFM_Read(0x13); /*Read length of received package*/

  RFM_Write(0x0D,RFM_Package_Location); /*Set SPI pointer to start of package*/

  for (i = RFM_Package_Length; i > 0; i--)
  {
    *RFM_Rx_Package = RFM_Read(0x00);
    RFM_Rx_Package++;
  }

  return RFM_Package_Length;
}

/*
*****************************************************************************************
* Description : Funtion that reads a register from the RFM and returns the value
*
* Arguments   : RFM_Address Address of register to be read
*
* Returns   : Value of the register
*****************************************************************************************
*/

unsigned char RFM_Read(unsigned char RFM_Address)
{
  unsigned char RFM_Data;

  //Set NSS pin low to start SPI communication
  digitalWrite(10,LOW);

  //Send Address
  SPI.transfer(RFM_Address);
  //Send 0x00 to be able to receive the answer from the RFM
  RFM_Data = SPI.transfer(0x00);

  //Set NSS high to end communication
  digitalWrite(10,HIGH);

  //Return received data
  return RFM_Data;
}

/*
*****************************************************************************************
* Description : Funtion that writes a register from the RFM
*
* Arguments   : RFM_Address Address of register to be written
*         RFM_Data    Data to be written
*****************************************************************************************
*/

void RFM_Write(unsigned char RFM_Address, unsigned char RFM_Data)
{
  //Set NSS pin Low to start communication
  digitalWrite(10,LOW);

  //Send Addres with MSB 1 to make it a writ command
  SPI.transfer(RFM_Address | 0x80);
  //Send Data
  SPI.transfer(RFM_Data);

  //Set NSS pin High to end communication
  digitalWrite(10,HIGH);
}
