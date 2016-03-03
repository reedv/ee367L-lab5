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
	// populate the manager links array with nonblocking  managerLinks
	netCreateConnections(& manager_links_array);

	/* Create links between nodes but not setting their end nodes */
	linkArrayType links_array;
	links_array.numlinks = NUMLINKS;
	// populate linkArrayType with nonblocking, unipipe links
	netCreateLinks(& links_array);

	/* Set the end nodes of the links
	 * CURRENTLY THERE ARE JUST 2 HOSTS LINKED TOGETHER
	 * TODO: Change to have 3 hosts connected to a switch and a manager connected to a host*/
	netSetNetworkTopology(& links_array);

	/* Create nodes and spawn their own processes, one process per node */
	hostState host_state;   /* The host's state */
	pid_t process_id;  		/* Process id */
	int host_physid; 		/* Physical ID of host */
	// init. each host
	for (host_physid = 0; host_physid < NUMHOSTS; host_physid++) {

	   process_id = fork();

	   if (process_id == -1) {
		  printf("Error: the fork() failed\n");
		  return 0;
	   }
	   else if (process_id == 0) {
		  /* The child process -- a host node */

		  /* Initialize host's state */
		  hostInit(&host_state, host_physid);

		  /* Initialize the host's managerLink connection to the manager */
		  // adds host's physid to the manager's array of connectable hosts
		  host_state.manLink = manager_links_array.links[host_physid];

		  /*
		   * Close all managerLink connections not incident to the host
		   * Also close the manager's side of connections to host
		   * (creates bi-link between host and manager using 2 oneway links)
		   */
		  netCloseConnections(& manager_links_array, host_physid);

		  /* Initialize the host's incident communication links */
		  int host_link_index;
		  // set host's link_out from linkArrayType
		  host_link_index = netHostOutLink(&links_array, host_physid); /* Host's OUTGOING link */
		  host_state.link_out = links_array.link[host_link_index];
		  // set host's link_in from linkArrayType
		  host_link_index = netHostInLink(&links_array, host_physid); /* Host's INCOMING link */
		  host_state.link_in = links_array.link[host_link_index];

		  /* Close all other links -- not incident to the host */
		  netCloseHostOtherLinks(&links_array, host_physid);


		  /* Go to the main loop of the host node */
		  hostMain(&host_state);
		  return 0;
	   }
	}

	/* The host process -- Manager interface */

	/*
	 * The manager is connected to the hosts and doesn't
	 * need the links between nodes
	 */

	/* Close all links between nodes */
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




