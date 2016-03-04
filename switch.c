/*
 * switch.c
 *
 *  Created on: Mar 1, 2016
 *      Author: reedvillanueva
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "main.h"
#include "link.h"
#include "switch.h"

#define MAXBUFFER 1000
#define TENMILLISEC 10000



void switchInit(switchState * sstate, int physid) {
	printf("** switch.c/switchInit: entered");
    sstate->physid = physid;
    sstate->numInLinks = 0;
    sstate->numOutLinks = 0;

    printf("** switch.c/switchInit: init queue and table");
    tableInit(&(sstate->forwardingTable));
    queueInit(&(sstate->packetQueue));
}

void tableInit(ForwardingTable * ftable) {
    ftable->size = 0;
}

void queueInit(PacketQueue * pqueue) {
    pqueue->size = 0;
    pqueue->head = 0;
    pqueue->tail = -1;
}



void switchMain(switchState * sstate)
{
    int outLink;            // Link to transmit packet on
    packetBuffer outPacket; // packet to be sent
    while(1) {
		switchStoreIncomingPackets(sstate);

        // If queue not empty, send next packet
        if(sstate->packetQueue.size != 0) {
            // Get packet from head of PacketQueue
            outPacket = queuePop(&(sstate->packetQueue));

            // Check ForwardingTable for outgoing link
            outLink = tableGetOutLink(&(sstate->forwardingTable), outPacket.dest_addr);

            // If incoming addr of packet to be sent exists
            if (outPacket.src_addr != -1) {
            	// send packet to dest_addr or to all
            	switchSendOutPacket(outPacket, outLink, sstate);
            }
        }

        // Sleep for 10 milliseconds
        usleep(TENMILLISEC);
    }
}

void switchStoreIncomingPackets(switchState* sstate) {
	printf("\n** switch.c/switchStoreIncomingPackets\n");
	int inLink_index;
	packetBuffer tmpPacket;
	// Check all incoming links for arriving packets
	for (inLink_index = 0; inLink_index < sstate->numInLinks; inLink_index++) {
		// Check link for packets
		linkReceive(&(sstate->inLinks[inLink_index]), &tmpPacket);
		// Put packet in PacketQueue
		queuePush(&(sstate->packetQueue), tmpPacket);
		// Update ForwardingTable
		tableUpdate(&(sstate->forwardingTable),
				tmpPacket.is_valid, tmpPacket.src_addr, inLink_index);
	}
}

void switchSendOutPacket(packetBuffer outPacket, int outLink, switchState* sstate) {
	printf("\n** switch.c/switchSendOutPacket\n");
	// If outgoing link from ForwardingTable exists
	if (outLink != -1) {
		// Send packet along the outgoing link
		linkSend(&(sstate->outLinks[outLink]), &outPacket);
	}
	// Else send to all links except for the incoming link packet was received on
	else {
		// Get source link of packet to be sent
		int inLink = tableGetOutLink(&(sstate->forwardingTable), outPacket.src_addr);
		// For all outgoing links
		int outLink_index;
		for (outLink_index = 0; outLink_index < sstate->numOutLinks;
				outLink_index++) {
			// Send on link so long as its not the incoming link
			if (outLink_index != inLink)
				linkSend(&(sstate->outLinks[outLink_index]), &outPacket);
		}
	}
}



/*
 * PacketQueue operations --------------------------------------------------------
 * */

void queuePush(PacketQueue * pqueue, packetBuffer packet) {
    if(!queueIsFull(pqueue)) {
        pqueue->tail = (pqueue->tail+1) % MAXQUEUE;  // tail circles back count from 0 when tail >= MAXQUEUE
        											 //    (avoids having to reset counter)
        pqueue->packets[pqueue->tail] = packet;
        pqueue->size++;
    }
    else {
        printf("Error: packetQueue is full\n");
    }
}

int queueIsFull(PacketQueue * pqueue) {
    if(pqueue->size == MAXQUEUE) return 1;
    else return 0;
}



packetBuffer queuePop(PacketQueue * pqueue)
{
    packetBuffer popped;
    if(!queueIsEmpty(pqueue)) {
        popped = pqueue->packets[pqueue->head];
        pqueue->head = (pqueue->head+1) % MAXQUEUE;	// head circles back count from 0 when head >= MAXQUEUE
        pqueue->size--;
        return popped;
    }
    else {
        printf("Error: packetQueue is empty\n");
    }  /*TODO: OK to return nothing here?
     	 Only used in switchMain and tableGetOutLink may take care of this there.*/
}

int queueIsEmpty(PacketQueue * pqueue) {
    if(pqueue->size == 0) return 1;
    else return 0;
}



// for debugging
void queueDisplay(PacketQueue * pqueue) {
    int i;

    printf("Source Address\tDest Address\tLength\tPayload\n");
    for(i=pqueue->head; i <= pqueue->tail; i++) {
        printf("\t%d\t\t%d\t%d\t%s\n",
        		pqueue->packets[i].src_addr,
				pqueue->packets[i].dest_addr,
				pqueue->packets[i].length,
				pqueue->packets[i].payload);
    }
}



/*
 * ForwardingTable operations --------------------------------------------------------
 * */

// Update the table
void tableUpdate(ForwardingTable * ftable, int valid,
				 int dest_addr, int linkOut) {
    int table_index;

    table_index = tableEntryIndex(ftable, dest_addr);

    // if index not yet in table
    if(table_index == -1)
        tableAddEntry(ftable, valid, dest_addr, linkOut);
    // else modify the entry with the specified dest_addr
    else
    	tableUpdateEntry(ftable, table_index, valid, linkOut);
}

void tableAddEntry(ForwardingTable * ftable, int valid,
				   int dest_addr, int linkOut) {
    TableEntry entry;

    entry.valid = valid;
    entry.dest_addr = dest_addr;
    entry.linkOut = linkOut;

    ftable->entries[ftable->size] = entry;
    ftable->size++;
}

// Update an existing entry in the table
void tableUpdateEntry(ForwardingTable * ftable, int table_index,
					  int valid, int linkOut) {
    ftable->entries[table_index].valid = valid;
    ftable->entries[table_index].linkOut = linkOut;
}



// Retrieve the value of the out link
int tableGetOutLink(ForwardingTable * ftable, int dest_addr) {
    int table_index;
    int linkOut;

    table_index = tableEntryIndex(ftable, dest_addr);

    if(table_index == -1)
        return table_index;
    else {
        linkOut = ftable->entries[table_index].linkOut;
        return linkOut;
    }
}

// Find the index of an entry in the table
int tableEntryIndex(ForwardingTable * ftable, int dest_addr) {
    int table_index;
    for(table_index=0; table_index < ftable->size; table_index++) {
        if(ftable->entries[table_index].dest_addr == dest_addr)
            return table_index;
    }
    return -1;
}



// for debugging
void tableDisplay(ForwardingTable * ftable) {
    printf("Valid\tDestination Address\tLink Out #\n");

    int i;
    for(i=0; i<ftable->size; i++)
    {
        printf("%d\t%d\t\t\t%d\n",
        		ftable->entries[i].valid,
				ftable->entries[i].dest_addr,
				ftable->entries[i].linkOut);
    }
}
