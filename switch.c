/*
 * switch.c
 *
 *  Created on: Mar 1, 2016
 *      Author: reedvillanueva
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "switch.h"
#include "utilities.h"
#include "link.h"

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 1000
#define PIPE_WRITE 1
#define PIPE_READ  0
#define TENMILLISEC 10000   /* 10 millisecond sleep */

void switchInit(switchState * sstate, int physid) {
	switchInitState(sstate, physid);

	initPacketQueue(sstate);
	intiForwardTable(sstate);
}

void switchInitState(switchState * sstate, int physid) {
	sstate->physid = physid;
	sstate->netaddr = physid; /* default address */
	sstate->nbraddr = EMPTY_ADDR;
}

void initPacketQueue(switchState * sstate) {
	sstate->queue.head_index = 0;
	sstate->queue.packets = 0;
}

void intiForwardTable(switchState * sstate) {
	sstate->table.row = 0;
	sstate->table.valid = 0;
	sstate->table.dest_addrs = 0;
	sstate->table.out_links = 0;

	sstate->table.sendPacketBuff.is_valid = 0;
	sstate->table.sendPacketBuff.new = 0;
}


/*
 * switchMain will run an inf. loop.
 * At each pass thru loop,
 * it checks all incoming links for arriving packets and pushPacketQueue(packet)
 * As each packet from popPacketQueue() is processed, the switch's forwardingTable is updated.
 * One packet is transmitted from the packetQueue every 10ms (if there are any packets to send)
 * */
void switchMain(switchState * sstate) {

	while(1) {
		switchCheckIncomingLinks(sstate);

	}
}

void switchCheckIncomingLinks(switchState* sstate) {
	int links;
	for (links = 0; links < sstate->queue.in_links.numlinks; links++) {
		packetBuffer tmpbuff;
		linkReceive(&(sstate->queue.in_links.link), &tmpbuff);

		/*
		* If there is a packet
		* and if the packet's destination address is the host's network address
		* then store the packet in the switch's packetQueue
		*/
	   if (tmpbuff.is_valid == 1 && tmpbuff.new == 1) {
		   pushPacketQueue(sstate, tmpbuff);
	   }
	}
}


void pushPacketQueue(switchState * sstate, packetBuffer packet) {
	/* Create a path to the file and then open it */
	if (hstate->rcvPacketBuff.is_valid == 0) {
	   printf("switch/pauchPacketQueue: Download aborted: the receive packet buffer is empty");
	   return;
	}

	if (hstate->main_dir_valid == 0) {
	   strcpy(replymsg, "Download aborted: the host's main directory is not yet chosen");
	   return;
	}

	char path[MAXBUFFER];  /* Path to the file */
	strcpy(path,"");
	strcat(path, hstate->main_dir);
	strcat(path, "/");
	strcat(path, fname);
	printf("host:  path to the file: %s\n",path);
	FILE * fp;
	fp = fopen(path,"wb");	// The character b in opentype has a standard meaning;
							//    it requests a binary stream rather than a text stream.


	/* Download the packet buffer payload into the file */
	if (hstate->rcvPacketBuff.new == 1) {
	   fwrite(hstate->rcvPacketBuff.payload,1,hstate->rcvPacketBuff.length,fp);
	   hstate->rcvPacketBuff.new=0;
	}

	/* Message sent to the manager */
	strcpy(replymsg, "Download successful");
	fclose(fp);
}

packetBuffer popPacketQueue(switchState * sstate) {

}











