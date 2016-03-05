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
#include "logger.h"

#define MAXBUFFER 1000
#define TENMILLISEC 10000



void switchInit(switchState * sstate, int physid) {
	LOG_PRINT("** switch.c/switchInit: entered\n");
    sstate->physid = physid;
    sstate->numInLinks = 0;
    sstate->numOutLinks = 0;

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

// store expected links in switch's outLinks and inLinks
void switchSetupLinks(switchState * sstate, linkArrayType * links_array) {
	int i,
	    k;

	// find all links in links_array that belong in switch's inLinks array
		for (i=0, k=0; i < links_array->numlinks; i++) {
		   // if link in link_array has dest matching switch's physid, add that link to switch's inLinks
		   int isLinkToSwitchId = links_array->link[i].uniPipeInfo.dest_physId == sstate->physid;
		   if (isLinkToSwitchId) {
			   LOG_PRINT("** switch.c/switchSetupLinks get inLinks: adding link with src_physid = %d and dest_physid = %d to inLinks\n",
					   links_array->link[i].uniPipeInfo.src_physId,
					   links_array->link[i].uniPipeInfo.dest_physId);
			   sstate->inLinks[k] = links_array->link[i];
			   sstate->numInLinks++;
			   k++;
		   }
		}

	// find all links in links_array that belong in switch's outLinks array
	for (i=0, k=0; i < links_array->numlinks; i++) {
	   // if link in has src matching switch's physid, add that link to switch's outLinks
	   int isLinkFromSwitchId = links_array->link[i].uniPipeInfo.src_physId == sstate->physid;
	   if (isLinkFromSwitchId) {
		   LOG_PRINT("** switch.c/switchSetupLinks get outLinks: adding link with src_physid = %d and dest_physid = %d to outLinks\n",
				   links_array->link[i].uniPipeInfo.src_physId,
				   links_array->link[i].uniPipeInfo.dest_physId);
		   sstate->outLinks[k] = links_array->link[i];
		   sstate->numOutLinks++;
		   k++;
	   }
	}

}



void switchMain(switchState * sstate) {
	LOG_PRINT("** switch.c/switchMain: entered\n");
    int outLink;            // Link to transmit packet on
    packetBuffer outPacket; // packet to be sent
    while(1) {
    	// scan incoming links for packets
		switchStoreIncomingPackets(sstate);

        // If queue not empty, send next packet
        if(sstate->packetQueue.size != 0) {
            // Get packet from head of PacketQueue
            outPacket = queuePop(&(sstate->packetQueue));

            // Check ForwardingTable for outgoing link
            outLink = tableGetOutLink(&(sstate->forwardingTable), outPacket.dest_addr);

            // If incoming addr of packet to be sent actually exists
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
	int i;
	packetBuffer tmpPacket;
	// Check all incoming links for arriving packets
	for (i = 0; i < sstate->numInLinks; i++) {
		// Check link for packets
		linkReceive(&(sstate->inLinks[i]), &tmpPacket);
		// Put packet in PacketQueue
		if (tmpPacket.is_valid == 1) {
			LOG_PRINT("** switch.c/switchStorIncomingPackets: pushing packet bound for dest=%d in queue\n", tmpPacket.dest_addr);
			queuePush(&(sstate->packetQueue), tmpPacket);

			// Update ForwardingTable
			tableUpdate(&(sstate->forwardingTable),
					tmpPacket.is_valid, tmpPacket.src_addr, i);
			tableDisplay(&(sstate->forwardingTable));
		}
	}
}

void switchSendOutPacket(packetBuffer outPacket, int outLink, switchState* sstate) {
	// If outgoing link from ForwardingTable actually exists
	if (outLink != -1) {
		// Send packet along the outgoing link
		LOG_PRINT("** switch.c/switchSendOutPacket: sending packet to specific dest=%d\n", outPacket.dest_addr);
		linkSend(&(sstate->outLinks[outLink]), &outPacket);
	}
	// Else send to all links except for the incoming link packet was received on
	else {
		LOG_PRINT("** switch.c/switchSendOutPacket: sending packet on all outLinks\n");
		// Get source link of packet to be sent (to avoid using as a destination for the packet)
		//    even if src_addr of packet not yet in table's destination addrs, then will be -1 and skipped
		int srcLink = tableGetOutLink(&(sstate->forwardingTable), outPacket.src_addr);

		int i;
		for (i = 0; i < sstate->numOutLinks; i++) {
			// Send on link so long as its not the incoming link
			if (i != srcLink)
				linkSend(&(sstate->outLinks[i]), &outPacket);
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
    }
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



// Retrieve the value of the out link for the given dest_addr of an outgoing packet in the table
int tableGetOutLink(ForwardingTable * ftable, int dest_addr) {
    int table_index;
    int linkOut;

    // find index in table with the given addr
    table_index = tableEntryIndex(ftable, dest_addr);
    LOG_PRINT("** switch.c/tableGetOutLink: table_index=%d for dest_addr=%d\n", table_index, dest_addr);

    if(table_index == -1)
        return table_index;
    else {
    	// if entry with given addr was found, return entry's linkOut number
        linkOut = ftable->entries[table_index].linkOut;
        return linkOut;
    }
}

// Find the index of an entry in the table with the given addr
int tableEntryIndex(ForwardingTable * ftable, int dest_addr) {
    int i;
    for(i=0; i < ftable->size; i++) {
        if(ftable->entries[i].dest_addr == dest_addr)
            return i;
    }
    // if entry w/ given addr not found
    return -1;
}



// for debugging
void tableDisplay(ForwardingTable * ftable) {
    LOG_PRINT("Valid\tDestinationAddress\tLinkOut#\n");

    int i;
    for(i=0; i<ftable->size; i++)
    {
        LOG_PRINT("%d\t\t%d\t\t\t\t\t%d\n",
        		ftable->entries[i].valid,
				ftable->entries[i].dest_addr,
				ftable->entries[i].linkOut);
    }
}
