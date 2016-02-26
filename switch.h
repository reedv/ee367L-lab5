/*
 * switch.h
 *
 *  Created on: Feb 25, 2016
 *      Author: reedvillanueva
 */

#ifndef SWITCH_H_
#define SWITCH_H_

#define SWITCH_LINKS 100
#define PACKET_BUFFER_SIZE 100

typedef struct {
	int rear;	// for popping from end of array used to implement queue
	packetBuffer queue[PACKET_BUFFER_SIZE];
} packetQueue;

typedef struct {
	int row;
	int valid[SWITCH_LINKS];
	int dest_addrs[SWITCH_LINKS];
	linkArrayType out_links[SWITCH_LINKS];
} forwardingTable;

typedef struct { // state of switching node
   int   physid;              /* physical id */
   int   netaddr;             /* host's network address */
   int   nbraddr;             /* network address of neighbor */
   packetQueue packets;
   forwardingTable table;
} switchState;

void initPacketQueue(packetQueue * queue);
void pushPacketBuffer(packetBuffer packet, packetQueue * queue);
packetBuffer popPacketQueue(packetQueue * queue);

#endif /* SWITCH_H_ */
