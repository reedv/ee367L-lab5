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

#define TENMILLISEC 10000


void switchInit(switchState * sstate, int physID)
{
    sstate->physId = physID;
    sstate->numInLinks = 0;
    sstate->numOutLinks = 0;
    tableInit(&(sstate->forwardingTable));
    queueInit(&(sstate->packetQueue));
}

void tableInit(ForwardingTable * ftable)
{
    ftable->size = 0;
}

void queueInit(PacketQueue * pqueue)
{
    pqueue->size = 0;
    pqueue->head = 0;
    pqueue->tail = -1;
}


void switchMain(switchState * sstate)
{
    int numInLinks;
    int numPackets;
    int numOutLinks;
    int packetCount = 0;    // Number of packets on link
    int outLink;            // Link to transmit packet on
    int inLink;             // Link incoming packet arrived on
    packetBuffer outPacket;
    packetBuffer packets[10];

    while(1)
    {
        // Check all incoming links for arriving packets
        for(numInLinks=0; numInLinks<sstate->numInLinks; numInLinks++)
        {
            // Check link for packets
            packetCount = linkReceive(&(sstate->inLinks[numInLinks]), packets);

            // For all incoming packets on link
            for(numPackets=0; numPackets < packetCount; numPackets++)
            {
                // Put in packet queue
                queuePush(&(sstate->packetQueue), packets[numPackets]);

                // Update forwarding table
                tableUpdate(&(sstate->forwardingTable),
                		packets[numPackets].is_valid,
						packets[numPackets].src_addr,
						numInLinks);
            }
        }

        // If queue is not empty, transmit a packet
        if(sstate->packetQueue.size != 0)
        {
            // Get packet from head of packet Queue
            outPacket = queuePop(&(sstate->packetQueue));

            // Check forwarding table for outgoing link
            outLink = tableGetOutLink(&(sstate->forwardingTable), outPacket.dest_addr);

            if (outPacket.src_addr != -1 )
            {
                // If out going link exists in table
                if(outLink != -1)
                {
                    // Transmit packet on the outgoing link
                    linkSend(&(sstate->outLinks[outLink]), &outPacket);
                }

                // Else send to all links except for the incoming one
                else
                {
                    // Get source link of packet
                    inLink = tableGetOutLink(&(sstate->forwardingTable), outPacket.src_addr);

                    // For all outgoing links
                    for(numOutLinks=0; numOutLinks<sstate->numOutLinks; numOutLinks++)
                    {
                        // Send on link if its not the incoming link
                        if(numOutLinks != inLink)
                            linkSend(&(sstate->outLinks[numOutLinks]), &outPacket);
                    }
                }
            }
        }

        // Sleep for 10 milliseconds
        usleep(TENMILLISEC);
    }
}







int queueIsEmpty(PacketQueue * pqueue)
{
    if(pqueue->size == 0)
        return 1;
    else return 0;
}

int queueIsFull(PacketQueue * pqueue)
{
    if(pqueue->size == MAXQUEUE)
        return 1;
    else return 0;
}

void queuePush(PacketQueue * pqueue, packetBuffer packet)
{
    if(!queueIsFull(pqueue))
    {
        pqueue->tail = (pqueue->tail+1) % MAXQUEUE;  // rather than having to reset tail whenever tail > MAXQUEUE
        pqueue->packets[pqueue->tail] = packet;
        pqueue->size++;
    }
    else
    {
        printf("Error: packetQueue is full\n");
    }
}

packetBuffer queuePop(PacketQueue * pqueue)
{
    packetBuffer deleted;
    if(!queueIsEmpty(pqueue))
    {
        deleted = pqueue->packets[pqueue->head];
        pqueue->head = (pqueue->head+1) % MAXQUEUE;	// rather than having to reset head whenever head > MAXQUEUE
        pqueue->size--;
        return deleted;
    }
    else
    {
        printf("Error: packetQueue is empty\n");
    }
}

void queueDisplay(PacketQueue * pqueue)
{
    int i;

    printf("Source Address\tDest Address\tLength\tPayload\n");

    for(i=pqueue->head; i <= pqueue->tail; i++)
    {
        printf("\t%d\t\t%d\t%d\t%s\n",
        		pqueue->packets[i].src_addr,
				pqueue->packets[i].dest_addr,
				pqueue->packets[i].length,
				pqueue->packets[i].payload);
    }
}







void tableAddEntry(ForwardingTable * ftable, int valid, int dest_addr, int linkOut)
{
    TableEntry newentry;
    newentry.valid = valid;
    newentry.dest_addr = dest_addr;
    newentry.linkOut = linkOut;
    ftable->entries[ftable->size] = newentry;
    ftable->size++;
}

// Find the index of an entry in the table
int tableEntryIndex(ForwardingTable * ftable, int dest_addr)
{
    int i;
    for(i=0; i<ftable->size; i++)
    {
        if(ftable->entries[i].dest_addr == dest_addr)
            return i;
    }
    return -1;
}

// Update an existing entry in the table
void tableUpdateEntry(ForwardingTable * ftable, int i, int valid, int linkOut)
{
    ftable->entries[i].valid = valid;
    ftable->entries[i].linkOut = linkOut;
}

// Update the table
void tableUpdate(ForwardingTable * ftable, int valid, int dest_addr, int linkOut)
{
    int i;

    i = tableEntryIndex(ftable, dest_addr);

    if(i == -1)
        tableAddEntry(ftable, valid, dest_addr, linkOut);
    else tableUpdateEntry(ftable, i, valid, linkOut);
}

// Retrieve the value of the out link
int tableGetOutLink(ForwardingTable * ftable, int dest_addr)
{
    int i;
    int linkOut;

    i = tableEntryIndex(ftable, dest_addr);

    if(i == -1)
        return i;

    else
    {
        linkOut = ftable->entries[i].linkOut;
        return linkOut;
    }
}

void tableDisplay(ForwardingTable * ftable)
{
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
