/*
 * WifiQueue library v1.7.5 (151129)
 *
 * Copyright 2015-2013 by M. Westenberg (mw12554@hotmail.com)
 *
 * License: GPLv3. See license.txt
 */

#ifndef Wifiqueue_h
#define Wifiqueue_h

#include <Arduino.h>

// We have to define how many entries are allowd in the Queue.
// As all entries have memory availble through malloc() there should not be a problem.
#define MAX_QUEUE 32

typedef void (*QueueCallback)();

// We need to use this structure for both sensors and devices.
// As a result we'll need to double up some felds
// without enlarging memory usage.
struct queueItem {
		union { uint32_t address; uint32_t gaddr; };
		union { uint8_t channel; uint8_t uaddr; };		// channel is used with sensors, uaddr for devices
		union { float value; char val[4]; };			// val can be "off", "on", 0-15
		char action[8];									// both sensors and device messages have action 
		char type[8];									// and type;
		union { char brand[8]; char cmd[8]; };
		union { char label[16]; char message[16]; };
};

//
// For internal use
//
class QueueLink {
	public:
		QueueLink *next;
		queueItem item;
		void init(queueItem itemIn, QueueLink *nextIn);		
};

class QueueChain {  
	public:
		// Add an  handler on interrupt queueNr. The callback is of the same type as	
		static void addQueue(queueItem item, QueueCallback callbackIn);
		static void delQueue(queueItem item, QueueCallback callbackIn);		
		//Set the queue mode for given queue number. 
		static void setMode(byte modeIn);
		static void printQueue();
		static int processQueue(queueItem *itemIn);
	private:
		static QueueLink *queue;
		static byte mode;
};
#endif