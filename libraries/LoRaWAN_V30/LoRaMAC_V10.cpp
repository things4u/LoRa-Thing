/******************************************************************************************
*
* File:        LoRaMAC_V10.cpp
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

/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/

#include "RFM95_V20.h"
#include "Encrypt_V30.h"
#include "LoRaMAC_V10.h"
#include "Waitloop_V10.h"

/*
*****************************************************************************************
* INCLUDE GLOBAL VARIABLES
*****************************************************************************************
*/

unsigned int Frame_Counter_Up = 0x0000;
extern unsigned char DevAddr[4];

/*
*****************************************************************************************
* FUNCTIONS
*****************************************************************************************
*/


/*
*****************************************************************************************
* Description : Function that handles one cycle of sending and receiving with the LoRaWAN protocol.
*               In this function the timing of receive slot 2 is handeld
*
* Arguments   : *Data_Tx pointer to the array of data that will be transmitted
*               *Data_Rx pointer to the array where the received data will be stored
*               Data_Length_Tx nuber of bytes to be transmitted
*               
* Returns     : Number of bytes received
*****************************************************************************************
*/
unsigned char LORA_Cycle(unsigned char *Data_Tx, unsigned char *Data_Rx, unsigned char Data_Length_Tx)
{
	unsigned char Data_Length_Rx;
  unsigned char i;

	//Time to wait in ms for receive slot 2
	unsigned char Receive_Delay_2 = 17;

	LORA_Send_Data(Data_Tx, Data_Length_Tx, Frame_Counter_Up);

	//Raise frame counter
	Frame_Counter_Up++;

  for(i = 0; i <= Receive_Delay_2; i ++ )
  {
    WaitLoop(100);
  }

	Data_Length_Rx = LORA_Receive_Data(Data_Rx);

	return Data_Length_Rx;
}

/*
*****************************************************************************************
* Description : Function contstructs a LoRaWAN package and sends it
*
* Arguments   : *Data pointer to the array of data that will be transmitted
*               Data_Length nuber of bytes to be transmitted
*               Frame_Counter_Up  Frame counter of upstream frames
*****************************************************************************************
*/
void LORA_Send_Data(unsigned char *Data, unsigned char Data_Length, unsigned int Frame_Counter_Up)
{
	//Define variables
	unsigned char i;

	//Direction of frame is up
	unsigned char Direction = 0x00;

	unsigned char RFM_Data[256];
	unsigned char RFM_Package_Length;

	unsigned char MIC[4];

	//Unconfirmed data up
	unsigned char Mac_Header = 0x40;

	unsigned char Frame_Control = 0x00;
	unsigned char Frame_Port = 0x01;

	//Encrypt the data
	Encrypt_Payload(Data, Data_Length, Frame_Counter_Up, Direction);

	//Build the Radio Package
	RFM_Data[0] = Mac_Header;

	RFM_Data[1] = DevAddr[3];
	RFM_Data[2] = DevAddr[2];
	RFM_Data[3] = DevAddr[1];
	RFM_Data[4] = DevAddr[0];

	RFM_Data[5] = Frame_Control;

	RFM_Data[6] = (Frame_Counter_Up & 0x00FF);
	RFM_Data[7] = ((Frame_Counter_Up >> 8) & 0x00FF);

	RFM_Data[8] = Frame_Port;

	//Set Current package length
	RFM_Package_Length = 9;

	//Load Data
	for(i = 0; i < Data_Length; i++)
	{
		RFM_Data[RFM_Package_Length + i] = Data[i];
	}

	//Add data Lenth to package length
	RFM_Package_Length = RFM_Package_Length + Data_Length;

	//Calculate MIC
	Calculate_MIC(RFM_Data, MIC, RFM_Package_Length, Frame_Counter_Up, Direction);

	//Load MIC in package
	for(i = 0; i < 4; i++)
	{
		RFM_Data[i + RFM_Package_Length] = MIC[i];
	}

	//Add MIC length to RFM package length
	RFM_Package_Length = RFM_Package_Length + 4;

	//Send Package
	RFM_Send_Package(RFM_Data, RFM_Package_Length);
}


/*
*****************************************************************************************
* Description : Function that handles received data. Checks the MIC and deconstructs the LoRaWAN package
*
* Arguments   : *Data pointer to the array where the received data will be stored
*               
* Returns     : Number of bytes received
*****************************************************************************************
*/
unsigned char LORA_Receive_Data(unsigned char *Data)
{
	unsigned char i;

	unsigned char Data_Length = 0x00;

	unsigned char RFM_Data[64];
	unsigned char RFM_Package_Length = 0x00;
	unsigned char RFM_Interrupt;

	unsigned char MIC[4];
	unsigned char MIC_Check;

	unsigned int Frame_Counter_Down;
	unsigned char Frame_Control;
	unsigned char Frame_Options_Length;

	unsigned char Data_Location;

	unsigned char Direction;

	message_t Message_Status = NO_MESSAGE;


	Message_Status = RFM_Receive();


	//if CRC ok breakdown package
	if(Message_Status == CRC_OK)
	{
		//Get Message
		RFM_Package_Length = RFM_Get_Package(RFM_Data);

		//Set Direction for downlink frame
		Direction = 0x01;

		//Get frame counter
		Frame_Counter_Down = RFM_Data[7];
		Frame_Counter_Down = (Frame_Counter_Down << 8) + RFM_Data[6];

		Calculate_MIC(RFM_Data,MIC,(RFM_Package_Length - 4),Frame_Counter_Down,Direction);

		MIC_Check = 0x00;

		for(i = 0; i < 4; i++)
		{
			if(RFM_Data[(RFM_Package_Length - 4) + i] == MIC[i])
			{
				MIC_Check++;
			}
		}

		if(MIC_Check == 0x04)
		{
			Message_Status = MIC_OK;
		}
		else
		{
			Message_Status = WRONG_MESSAGE;
		}
	}

	//if MIC is OK then decrypt the data
	if(Message_Status == MIC_OK)
	{
		Data_Location = 8;

		//Get frame control
		Frame_Control = RFM_Data[5];

		//Get length of frame options field
		Frame_Options_Length = (Frame_Control & 0x0F);

		//Add length of frame options field to data location
		Data_Location = Data_Location + Frame_Options_Length;

		//Check if ther is data in the package
		if(RFM_Package_Length == (Data_Location + 4))
		{
			Data_Length = 0x00;
		}
		else
		{
			Data_Length = (RFM_Package_Length - Data_Location -1 -4);
			Data_Location = (Data_Location + 1);
		}

		//Decrypt the data
		if(Data_Length != 0x00)
		{
			Encrypt_Payload(&RFM_Data[Data_Location], Data_Length, Frame_Counter_Down, Direction);

			Message_Status = MESSAGE_DONE;
		}
	}

	if(Message_Status == MESSAGE_DONE)
	{
		for(i = 0; i < Data_Length; i++)
		{
			Data[i] = RFM_Data[Data_Location + i];
		}
	}

	if(Message_Status == WRONG_MESSAGE)
	{
		Data_Length = 0x00;
	}

	return Data_Length;
}
