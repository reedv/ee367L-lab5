/*
 * This is the source code of functions that will
 *
 *   - Set up and close links between nodes.  
 *   - Set up and close connections between the manager and hosts.
 *
 * In the program, connections are pipes, and links can be
 * pipes (and, in the future, sockets).
 *
 * Initially, all the connections and links are created.
 * But as the processes are forked, individual processes
 * will not need all the connections and links.  So they
 * close the unnecessary ones.
 *
 * The connections and link are nonblocking, which means
 * that they can be accessed but will not stop a process.
 * 
 * For example, if a process is reading a blocking pipe
 * that doesn't have enough bytes (e.g., it's empty) then
 * the process will wait until the pipe has sufficient
 * bytes.  So the process can get stuck.
 * 
 * On the other hand if the pipe is nonblocking then 
 * whenever a process reads a pipe that doesn't have enough
 * bytes then it continues processing and goes on to
 * do something else.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 1000
#define PIPE_READ  0
#define PIPE_WRITE 1 

/* 
 * Create nonblocking connections(pipes) between manager and hosts  
 * assuming host physical IDs are 0, 1, ....
 */
void netCreateConnections(manLinkArrayType * manLinkArray) 
{
	int link_index;
	int pflag;
	// for the expected size of manLinkArray, populate with nonblocking toHost and fromHost manLinks
	for (link_index=0; link_index < manLinkArray->numlinks; link_index++) {
	   // create filedes links toHost and fromHost
	   if (pipe(manLinkArray->links[link_index].toHost) < 0 || pipe(manLinkArray->links[link_index].fromHost) < 0) {
		  printf("Creating pipe failed\n");
		  return;
	   }

	   /* Set the pipes to nonblocking */
	   const int in_filedes = 0,
			   	 out_filedes = 1;
	   pflag = fcntl(manLinkArray->links[link_index].toHost[in_filedes], F_GETFL);
	   fcntl(manLinkArray->links[link_index].toHost[in_filedes], F_SETFL, pflag | O_NONBLOCK);

	   pflag = fcntl(manLinkArray->links[link_index].toHost[out_filedes],F_GETFL);
	   fcntl(manLinkArray->links[link_index].toHost[out_filedes], F_SETFL, pflag | O_NONBLOCK);

	   pflag = fcntl(manLinkArray->links[link_index].fromHost[in_filedes],F_GETFL);
	   fcntl(manLinkArray->links[link_index].fromHost[in_filedes], F_SETFL, pflag | O_NONBLOCK);

	   pflag = fcntl(manLinkArray->links[link_index].fromHost[out_filedes],F_GETFL);
	   fcntl(manLinkArray->links[link_index].fromHost[out_filedes], F_SETFL, pflag | O_NONBLOCK);
	}
}

/* 
 * Creates links to be used between nodes
 * but does not set the nodes for the ends of the links
 */
void netCreateLinks(linkArrayType * linkArray)
{ 
	// populate linkArray with links
	int i;
	for (i=0; i < linkArray->numlinks; i++) {
	   // set link info
	   linkArray->link[i].linkID = i;
	   linkArray->link[i].linkType = UNIPIPE;
	   linkArray->link[i].uniPipeInfo.pipeType = NONBLOCKING;

	   // set pipe filedes for the link
	   linkCreate(&(linkArray->link[i]));
	}
}

/* 
 * Close all connections except
 * the outgoing connection from the host to manager
 * and the incoming connection from the manager to host.
 * (creates single bi-link using 2 oneway links)
 */
void netCloseConnections(manLinkArrayType *  manLinkArray, int hostid)
{
	int manLink_index;

	/* Close all connections not incident to the host */
	for (manLink_index=0; manLink_index < manLinkArray->numlinks; manLink_index++) {
	   if (manLink_index != hostid) {
		  close(manLinkArray->links[manLink_index].toHost[0]);
		  close(manLinkArray->links[manLink_index].toHost[1]);

		  close(manLinkArray->links[manLink_index].fromHost[0]);
		  close(manLinkArray->links[manLink_index].fromHost[1]);
	   }
	}

	/* Close manager's side of the connection: from host to manager */
	close(manLinkArray->links[hostid].fromHost[PIPE_READ]);

	/* Close manager's side of the connection: from manager to host */
	close(manLinkArray->links[hostid].toHost[PIPE_WRITE]);
}

/*
 * Sets the end nodes incident on the links in the linkArray
 * CURRENTLY SET UP FOR TESTING 3 HOSTS CONNECTED TO A SWITCH
 */
void netSetNetworkTopology(linkArrayType * linkArray)
{
	linkArray->link[0].uniPipeInfo.src_physId = 0;   // link0 comes from host0
	linkArray->link[0].uniPipeInfo.dest_physId = 3;  //       goes to switch3

	linkArray->link[1].uniPipeInfo.src_physId = 3;   // link1 comes from switch3
	linkArray->link[1].uniPipeInfo.dest_physId = 0;  //       goes to host0


	linkArray->link[2].uniPipeInfo.src_physId = 1;   // link2 comes from host1
	linkArray->link[2].uniPipeInfo.dest_physId = 3;  //       goes to switch3

	linkArray->link[3].uniPipeInfo.src_physId = 3;   // link3 comes from switch3
	linkArray->link[3].uniPipeInfo.dest_physId = 1;  //       goes to host1


	linkArray->link[4].uniPipeInfo.src_physId = 2;   // link4 comes from host2
	linkArray->link[4].uniPipeInfo.dest_physId = 3;  //       goes to switch3

	linkArray->link[5].uniPipeInfo.src_physId = 3;   // link5 comes from switch3
	linkArray->link[5].uniPipeInfo.dest_physId = 2;  //       goes to host2

	// take linkarray, srcarray, destarray
	// for each link i=0 < linkarray->numlinks and k=0
	// 		set link[i]'s src_id = srcarray[k]
	//		set link[i]'s dest_id = destarray[k]
	// 		set link[i+1]'s src_id = destarray[k]
	//		set link[i+1]'s dest_id = srcarray[k]
	//		i += 2
	//		k++
	//
	// where srcarray is array of src_physIds for each link from link0 to linkN
	//       destarray is array of dest_physIds for each link from link0 to linkN

}

/*
 * Find host's outgoing link and return its index
 * from the link array
 */
int netHostOutLink(linkArrayType * linkArray, int hostid) 
{
	int i;
	int index;

	index = linkArray->numlinks;

	for (i=0; i < linkArray->numlinks; i++) {
	   /* Store index if the outgoing link is found */
	   if (linkArray->link[i].uniPipeInfo.src_physId == hostid)
		  index = i;
	}
	if (index == linkArray->numlinks)
	   printf("Error:  Can't find outgoing link for host\n");
	return index;
}

/*
 * Find host's incoming link and return its index
 * from the link array
 */
int netHostInLink(linkArrayType * linkArray, int hostid) 
{
	int i;
	int index;

	index = linkArray->numlinks;

	for (i=0; i<linkArray->numlinks; i++) {
	   /* Store index if the incoming link is found */
	   if (linkArray->link[i].uniPipeInfo.dest_physId == hostid)
		   index = i;
	}

	if (index == linkArray->numlinks)
	   printf("Error:  Can't find outgoing link for host\n");
	return index;
}

/*
 * Close links not connected to the host
 */
void netCloseNonincidentLinks(linkArrayType * linkArray, int hostid)
{
	int i;

	for (i=0; i < linkArray->numlinks; i++) {
	   if (linkArray->link[i].uniPipeInfo.src_physId != hostid)
		  close(linkArray->link[i].uniPipeInfo.pipe_filedes[PIPE_WRITE]);
	   if (linkArray->link[i].uniPipeInfo.dest_physId != hostid)
		  close(linkArray->link[i].uniPipeInfo.pipe_filedes[PIPE_READ]);
	}
}

/* Close all links*/
void netCloseLinks(linkArrayType * linkArray) 
{
	int link_index;
	for (link_index=0; link_index<linkArray->numlinks; link_index++)
	   linkClear(&(linkArray->link[link_index]));
}


/* Close the host's side of a connection between hosts and manager */
void netCloseManConnections(manLinkArrayType * manLinkArray)
{
	int i;
	for (i=0; i < manLinkArray->numlinks; i++) {
	   close(manLinkArray->links[i].toHost[PIPE_READ]);     // pipe only to send toHost
	   close(manLinkArray->links[i].fromHost[PIPE_WRITE]);  // pipe only to rcv fromHost
	}
}


void netCloseAllManConnections(manLinkArrayType * manLinkArray) {
	int i;
	for (i=0; i < manLinkArray->numlinks; i++) {
	   close(manLinkArray->links[i].toHost[PIPE_READ]);
	   close(manLinkArray->links[i].toHost[PIPE_WRITE]);

	   close(manLinkArray->links[i].fromHost[PIPE_READ]);
	   close(manLinkArray->links[i].fromHost[PIPE_WRITE]);
	}
}


