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
#include "logger.h"

#define EMPTY_ADDR  0xffff  /* Indicates the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 1000
#define PIPE_WRITE 1 
#define PIPE_READ  0


void mainSetTopology(int src_ids[], int dest_ids[],
		int * numhosts, int * numswitches, int * numlinks, char * file);

int main(int argc, char * argv[]) {
	// get the src_ids and dest_ids for each link in the network
	int src_Ids[MAX_TOPOLOGY];
	int dest_Ids[MAX_TOPOLOGY];
	int numhosts,
		numswitches,
		numlinks;
	mainSetTopology(src_Ids, dest_Ids,		// recall arrays are passed by ref.
			&numhosts, &numswitches, &numlinks, argv[1]);

	/*
	 * Create nonblocking (pipes) between manager and hosts
	 * assuming that hosts have physical IDs 0, 1, ... (that is, indexed from 0)
	 */
	manLinkArrayType manager_links_array;
	// set the number of hosts manager can connect to
	manager_links_array.numlinks = numhosts;
	// for expected number of hosts, populate the manager links array with managerLinks
	netCreateConnections(& manager_links_array);

	/* Create links to connect nodes but not actually setting their incident nodes */
	linkArrayType links_array;
	links_array.numlinks = numlinks;
	// for expected number of links, populate linkArrayType with links
	netCreateLinks(& links_array);

	/* Set the expected end nodes of the links */
	netSetNetworkTopology(& links_array, src_Ids, dest_Ids);
	linkDisplay(& links_array);
	usleep(10000);  // to give time to fully print list before spawning processes

	/* Create nodes and spawn their own processes, one process per node */
	hostState host_state;   /* The host's state */
	switchState switch_state;
	pid_t process_id;  		/* Process id */
	int physid; 		/* Physical ID of host */
	// init. each host
	for (physid = 0; physid < numhosts+numswitches; physid++) {
	   LOG_PRINT("** main.c process-spawning loop physid = %d \n", physid);
	   process_id = fork();

	   if (process_id == -1) {
		  printf("Error: the fork() failed\n");
		  return 0;
	   }
	   else if (process_id == 0) {
		   /* The child process */


		   // init. all hosts
		   if (physid < numhosts) {
			   LOG_PRINT("** main.c creating host process: physid = %d\n", physid);

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
			  int k;
			  // set host's link_out from linkArrayType
			  k = netHostOutLink(&links_array, physid); /* Host's OUTGOING link (if any) */
			  host_state.link_out = links_array.link[k];
			  LOG_PRINT("** main.c host%d: link_out dest_physid = %d\n",
					  physid, host_state.link_out.uniPipeInfo.dest_physId);
			  // set host's link_in from linkArrayType
			  k = netHostInLink(&links_array, physid); /* Host's INCOMING link (if any) */
			  host_state.link_in = links_array.link[k];
			  LOG_PRINT("** main.c host%d: link_in src_physid = %d\n",
					  physid, host_state.link_out.uniPipeInfo.src_physId);
			  LOG_PRINT("\n\n");

			  /* Close all other links -- not incident to the host */
			  netCloseNonincidentLinks(&links_array, physid);


			  /* Go to the main loop of the host node */
			  hostMain(&host_state);

		   }
		   // init. all switches: CURRENTLY HARDCODED TO SETUP A SINGLE SWITCH WITH 3 BI-CONNECTIONS
		   else {
			   LOG_PRINT("** main.c creating switch process: physid = %d\n", physid);

			   // inti. switch's state
			   switchInit(&switch_state, physid);

			   LOG_PRINT("** main.c creating switch process: init. switch links\n");
			   // setup switch's incoming and outgoing links
			   switchSetupLinks(&switch_state, &links_array);

			   LOG_PRINT("** main.c creating switch process: closing nonincidental links\n");
			   // close links not incident to the switch
			   netCloseNonincidentLinks(&links_array, physid);

			   LOG_PRINT("** main.c creating switch process: closing manager links\n");
			   // close manger links, since switches never connect to them
			   netCloseAllManConnections(&manager_links_array);

			   LOG_PRINT("** main.c creating switch process: starting switchMain loop\n");
			   // go to main loop of switch node
			   switchMain(&switch_state);
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

void mainSetTopology(int src_ids[], int  dest_ids[],
	int * numhosts, int * numswitches, int * numlinks, char file[]) {
	LOG_PRINT("** mainSetTopology: received topology file named %s\n", file);
	const int MAX_LINE = 100;
	char word[MAX_LINE+1];
	char line[MAX_LINE];
	FILE * topology;

	*numhosts = -1;
	*numswitches = -1;
	*numlinks = -1;

	topology = fopen(file, "r");

	if (topology != NULL)
	{
		LOG_PRINT("** mainSetTopology: topology file named %s NOT NULL\n", file);
		// get numhosts from 1st line in file
		fgets(line, sizeof line, topology);
		findWord(word, line, 1);
		*numhosts = ascii2Int(word);
		// get numswitches from 2nd line in file
		fgets(line, sizeof line, topology);
		findWord(word, line, 1);
		*numswitches = ascii2Int(word);
		// get numlinks from 3rd line in file
		//    multiply numlinks by 2 since each bi-link is made of 2 uni-links
		fgets(line, sizeof line, topology);
		findWord(word, line, 1);
		*numlinks = 2 * ascii2Int(word);
		LOG_PRINT("** mainSetTopology: numhosts=%d, numswitches=%d, numlinks=%d\n",
				*numhosts, *numswitches, *numlinks);

		// get the src and dest addrs for the links from the remaining lines in the file
		int i;
		for(i=0; fgets(line, sizeof line, topology) != NULL; i++) {
			findWord(word, line, 1);
			src_ids[i] = ascii2Int(word);
			LOG_PRINT("** mainSetTopology: src_ids[%d]=%d\n", i, ascii2Int(word));

			findWord(word, line, 2);
			dest_ids[i] = ascii2Int(word);
			LOG_PRINT("** mainSetTopology: dest_ids[%d]=%d\n", i, ascii2Int(word));
		}

		fclose(topology);
	}
	else {
		LOG_PRINT("** mainSetTopology: error opening topology file named %s\n", file);
		printf("Error opening the file");
		exit(1);
	}

	// check validity of topology file inputs
	if (numhosts < 0 || numswitches < 0 || numlinks < 0 ) {
		LOG_PRINT("** mainSetTopology: error, topology file named %s has invalid values for either numhosts, numswitches, or numlinks\n", file);
		printf("The file has invalid inputs");
		exit(0);
	}
}




