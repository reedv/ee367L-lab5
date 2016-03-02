/*
 * switch.h
 *
 *  Created on: Feb 25, 2016
 *      Author: reedvillanueva
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#define MAXQUEUE 5
#define MAXTABLE 10


typedef struct {
	int head;
	int tail;
	int size;
	packetBuffer packets[MAXQUEUE];
} PacketQueue;


/* Data types for forwarding table */
typedef struct {
    int valid;
    int dest_addr;
    int linkOut;
} TableEntry;

typedef struct{
    int size;
    TableEntry entries[MAXTABLE];
} ForwardingTable;


/* Data types for switch */
typedef struct {
    int physId;
    int numInLinks;
    int numOutLinks;
    LinkInfo * inLinks;      	// Incoming communication link
    LinkInfo * outLinks;     	// Outgoing communication link
    ForwardingTable forwardingTable;
    PacketQueue packetQueue;
} switchState;


void queueInit(PacketQueue * pqueue);
int queueIsEmpty(PacketQueue * pqueue);
int queueIsFull(PacketQueue * pqueue);
void queuePush(PacketQueue * pqueue, packetBuffer packet);
packetBuffer queuePop(PacketQueue * pqueue);
void queueDisplay(PacketQueue * pqueue);


void tableInit(ForwardingTable * ftable);
void tableAddEntry(ForwardingTable * ftable, int valid, int dest_addr, int linkOut);
int tableEntryIndex(ForwardingTable * ftable, int dest_addr);
void tableUpdateEntry(ForwardingTable * ftable, int valid, int dstaddr, int linkOut);
void tableUpdate(ForwardingTable * ftable, int valid, int dest_addr, int linkOut);
int tableGetOutLink(ForwardingTable * ftable, int dest_addr);
void tableDisplay(ForwardingTable * ftable);


void switchInit(switchState * sstate, int physid);
void switchMain(switchState * sstate);

#endif /* SWITCH_H_ */
