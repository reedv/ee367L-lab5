/*
 * switch.h
 *
 *  Created on: Feb 25, 2016
 *      Author: reedvillanueva
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#define SWITCH_LINKS 100
#define PACKET_BUFFER_SIZE 5  // arbitrary for improvement 1

typedef struct {
	int head_index;	// for popping from REAR of array used to implement queue
	packetBuffer packets[PACKET_BUFFER_SIZE];  // all received packets
	linkArrayType in_links[SWITCH_LINKS];
} packetQueue;

typedef struct {
	int row;	// used to refer to a valid-dest-out 'row' in the table
	int valid[SWITCH_LINKS];
	int dest_addrs[SWITCH_LINKS];
	linkArrayType out_links[SWITCH_LINKS];
	packetBuffer sendPacketBuff;	// packet to be sent by table
} forwardingTable;

typedef struct { // state of switching node
   int   physid;              /* physical id */
   int   netaddr;             /* host's network address */
   int   nbraddr;             /* network address of neighbor */
   packetQueue queue;
   forwardingTable table;
} switchState;

void pushPacketQueue(switchState * sstate, packetBuffer packet);
packetBuffer popPacketQueue(switchState * sstate);


void switchInit(switchState * sstate, int physid);
void switchInitState(switchState * sstate, int physid);
void initPacketQueue(switchState * sstate);
void intiForwardTable(switchState * sstate);



void switchMain(switchState * sstate);
/*
 * switchMain will run an inf. loop.
 * At each pass thru loop, it checks all incoming links for arriving packets and pushPacketQueue(packet)
 * As each packet from popPacketQueue() is processed, the switch's forwardingTable is updated.
 * One packet is transmitted from the packetQueue every 10ms (if there are any packets to send)
 * */

#endif /* SWITCH_H_ */
