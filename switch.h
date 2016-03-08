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
    int linkOutNum;
} TableEntry;

typedef struct{
    int size;
    TableEntry entries[MAXTABLE];
} ForwardingTable;


/* Data types for switch */
typedef struct {
    int physid;
    int numInLinks;
    int numOutLinks;
    LinkInfo inLinks[10];  // CURRENT SIZES FOR LINK ARRAYS ARE ARBITRARY AND ONLT FOR TESTING
    LinkInfo outLinks[10];
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
void tableUpdateEntry(ForwardingTable * ftable, int table_index, int valid, int linkOut);
void tableUpdate(ForwardingTable * ftable, int valid, int dest_addr, int linkOut);
int tableGetOutLink(ForwardingTable * ftable, int dest_addr);
void tableDisplay(ForwardingTable * ftable);


void switchInit(switchState * sstate, int physid);
void switchSetupLinks(switchState * sstate, linkArrayType * links_array);
void switchMain(switchState * sstate);
void switchStoreIncomingPackets(switchState* sstate);
void switchSendOutPacket(packetBuffer outPacket, int outLink, switchState* sstate);

#endif /* SWITCH_H_ */
