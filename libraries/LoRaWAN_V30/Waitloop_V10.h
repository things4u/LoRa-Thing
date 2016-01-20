/******************************************************************************************
*
* File:        Waitloop_V10.h
* Author:      Gerben den Hartog
* Compagny:    Ideetron B.V.
* Website:     http://www.ideetron.nl/LoRa
* E-mail:      info@ideetron.nl
******************************************************************************************/
/****************************************************************************************
*
* Created on: 			  20-11-2015
* Supported Hardware: ID150119-02 Nexus board with RFM95
*
* History:
*
* Firmware version: 1.0
* First version
****************************************************************************************/

#ifndef WAITLOOP_V10_H
#define WAITLOOP_V10_H

/*
*****************************************************************************************
* FUNCTION PROTOTYPES
*****************************************************************************************
*/

void WaitLoop_Init();
void WaitLoop(unsigned char ms);

#endif
