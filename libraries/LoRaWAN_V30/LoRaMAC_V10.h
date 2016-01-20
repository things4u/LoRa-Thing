/******************************************************************************************
*
* File:        LoRaMAC_V10.h
* Author:      Gerben den Hartog
* Compagny:    Ideetron B.V.
* Website:     http://www.ideetron.nl/LoRa
* E-mail:      info@ideetron.nl
******************************************************************************************/
/****************************************************************************************
*
* Created on: 			13-11-2015
* Supported Hardware: ID150119-02 Nexus board with RFM95
*
* Firmware version: 1.0
* First version
****************************************************************************************/

#ifndef LORAMAC_V10_H
#define LORAMAC_V10_H

/*
*****************************************************************************************
* FUNCTION PROTOTYPES
*****************************************************************************************
*/

unsigned char LORA_Cycle(unsigned char *Data_Tx, unsigned char *Data_Rx, unsigned char Data_Length_Tx);
void LORA_Send_Data(unsigned char *Data, unsigned char Data_Length, unsigned int Frame_Counter_Up);
unsigned char LORA_Receive_Data(unsigned char *Data);

#endif
