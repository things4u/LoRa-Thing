/*
 * Queue library v1.7.5 (151129)
 *
 * Copyright 2015-2015 by M Westenberg (mw12554@hotmail.com)
 *
 * License: GPLv3. See license.txt
 */

#include <wifiQueue.h>
     
void QueueLink::init(queueItem itemIn, QueueLink *nextIn) {
	item = itemIn;					// Should copy the complete structure
	next = nextIn;
}

QueueLink *QueueChain::queue = NULL;
byte QueueChain::mode = 0;

// OK
void QueueChain::setMode(byte modeIn) {     
    mode = modeIn;
}

// Add an item to the queue. Including the function needed to process....
// We need to add members at the backend of the queue for first in first out operation
//
void QueueChain::addQueue(queueItem item, QueueCallback callback) {
  if (queue == NULL) {
	    queue = (QueueLink *) malloc(sizeof(QueueLink));
		queue->init(item, NULL);
  } 
  else {
	QueueLink *walk = queue;
	while (walk->next != NULL) {
		walk = walk->next;
	}
	walk->next = (QueueLink *) malloc(sizeof(QueueLink)); // malloc instead of new, due to the lack of new / delete support in AVR-libc
	walk = walk->next;
	walk->init(item, NULL); 
  }
}

// XXX Delete an item from chain
void QueueChain::delQueue(queueItem item, QueueCallback callback) {
  QueueLink *prevLink = queue; // Note: the chain-array is NULL initialized, so the first time prevLink is indeed NULL
  queue = (QueueLink *) malloc(sizeof(QueueLink)); // malloc instead of new, due to the lack of new / delete support in AVR-libc
  queue->init(item, prevLink);      
}

// This function is non descruptive for the queue
void QueueChain::printQueue() {
	QueueLink *q;
	q = queue;
	if (q != NULL) Serial.println(F("Print Sensor QUEUE: "));
	while (q != NULL) {
		Serial.print(F("\tA: ")); Serial.print(q->item.address) ;
		Serial.print(F(", C: ")); Serial.print(q->item.channel) ;
		Serial.print(F(", B: ")); Serial.print(q->item.brand) ;
		Serial.print(F(", L: ")); Serial.print(q->item.label) ;
		Serial.print(F(", V: ")); Serial.print(q->item.value) ;
		Serial.println();
		q = q->next;
	}
}

// processQueue starts at the front-end of the queue (oldest members)
// We have to be carefull when adding to the queue to protect the queue head.
// We use a pointer else we will not be be to output.
// Memory allocation is done by calling function!
// After returning the first item, the item is removed from the queue
// and its memory is released
int QueueChain::processQueue(queueItem *itemIn) {
	QueueLink *ql;
	if (queue != NULL) {
		// Lock queue for a small time
		ql = queue;
		// If exactly at this moment the interrupt routine kicks in
		queue = queue->next;			// Now we're ready for new queue
		// From this moment on there is no danger
		// output the item
		*itemIn = ql->item;
		// Delete node
		free(ql);
		// return value
		return (0);
	}
	return (-1);
}
