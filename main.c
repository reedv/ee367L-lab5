#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"
#include "switch.h"
#include "net.h"

#define EMPTY_ADDR  0xffff  /* Indicates the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 1000
#define PIPE_WRITE 1 
#define PIPE_READ  0

int main()
{
	/*
	 * Create nonblocking (pipes) between manager and hosts
	 * assuming that hosts have physical IDs 0, 1, ... (that is, indexed from 0)
	 */
	manLinkArrayType manager_links_array;
	// set the number of hosts manager can connect to
	manager_links_array.numlinks = NUMHOSTS;
	// for expected number of hosts, populate the manager links array with managerLinks
	netCreateConnections(& manager_links_array);

	/* Create links to connect nodes but not actually setting their incident nodes */
	linkArrayType links_array;
	links_array.numlinks = NUMLINKS;
	// for expected number of links, populate linkArrayType with links
	netCreateLinks(& links_array);

	/* Set the end nodes of the links
	 * TODO: Change to have 3 hosts connected to a switch and a manager connected to a host*/
	netSetNetworkTopology(& links_array);

	/* Create nodes and spawn their own processes, one process per node */
	hostState host_state;   /* The host's state */
	switchState switch_state;
	pid_t process_id;  		/* Process id */
	int physid; 		/* Physical ID of host */
	// init. each host
	for (physid = 0; physid < NUMHOSTS+NUMSWITCHES; physid++) {

	   process_id = fork();

	   if (process_id == -1) {
		  printf("Error: the fork() failed\n");
		  return 0;
	   }
	   else if (process_id == 0) {
		   /* The child process -- inti. a host node and go to hostMain loop*/

		   // init. all hosts
		   if (physid < NUMHOSTS) {

		  /* Initialize host's state */
		  hostInit(&host_state, physid);

		  /* Initialize the host's managerLink connection to the manager */
		  // adds host's physid to the manager's array of connectable hosts
		  host_state.manLink = manager_links_array.links[physid];

		  /*
		   * Close all managerLink connections not incident to the host
		   * Also close the manager's side of connections to host
		   * (creates bi-link between host and manager using 2 oneway links)
		   */
		  netCloseConnections(& manager_links_array, physid);

		  /* Initialize the host's incident communication links */
		  int host_link_index;
		  // set host's link_out from linkArrayType
		  host_link_index = netHostOutLink(&links_array, physid); /* Host's OUTGOING link (if any) */
		  host_state.link_out = links_array.link[host_link_index];
		  // set host's link_in from linkArrayType
		  host_link_index = netHostInLink(&links_array, physid); /* Host's INCOMING link (if any) */
		  host_state.link_in = links_array.link[host_link_index];

		  /* Close all other links -- not incident to the host */
		  netCloseHostOtherLinks(&links_array, physid);


		  /* Go to the main loop of the host node */
		  hostMain(&host_state);
		   }
		   // init. all switches: CURRENTLY HARDCODED TO SETUP A SINGLE SWITCH WITH 3 BI-CONNECTIONS
		   else {
			   // inti. switch's state
			   switchInit(&switch_state, physid);

			   // inti. switch's incident communication links
			   switch_state.numInLinks = 3;
			   switch_state.numOutLinks = 3;

			   int links_index,
			   	   switch_links_index;
			   // find all links in links_array that belong in switch's outLinks array
			   for (links_index=0, switch_links_index=0; links_index < links_array.numlinks; links_index++) {
			   	   /* Store index if an outgoing link is found */
			   	   if (links_array.link[links_index].uniPipeInfo.src_physId == physid) {
			   		   switch_state.outLinks[switch_links_index] = links_array.link[links_index];
			   		   switch_links_index++;
			   	   }
			   	}
			   // find all links in links_array that belong in switch's inLinks array
			   for (links_index=0, switch_links_index=0; links_index < links_array.numlinks; links_index++) {
				   /* Store index if an incoming link is found */
				   if (links_array.link[links_index].uniPipeInfo.dest_physId == physid) {
					   switch_state.inLinks[switch_links_index] = links_array.link[links_index];
					   switch_links_index++;
				   }
				}

			   // close links not incident to the switch
			   netCloseHostOtherLinks(&links_array, physid);
		   }

		  return 0;
	   }
	}

	/* The parent process -- Manager interface */

	/*
	 * The manager is connected to the hosts and doesn't
	 * need the links between nodes
	 */
	/* Close all links between nodes in this process */
	netCloseLinks(&links_array);

	/* Close the host's side of connections between a host and manager */
	netCloseManConnections(&manager_links_array);

	/* Go to main loop for the manager */
	managerMain(& manager_links_array);

	/*
	 * We reach here if the user types the "q" (quit) command.
	 * Now if we don't do anything, the child processes will continue even
	 * after we terminate the parent process.  That's because these
	 * child proceses are running an infinite loop and do not exit
	 * properly.  Since they have no parent, and no way of controlling
	 * them, they are called "zombie" processes.  Actually, to get rid
	 * of them you would list your processes using the LINUX command
	 * "ps -x".  Then kill them one by one using the "kill" command.
	 * To use the kill the command just type "kill" and the process ID (PID).
	 *
	 * The following system call will kill all the children processes, so
	 * that saves us some manual labor
	 */
	kill(0, SIGKILL); /* Kill all processes */
	return 0;
}




