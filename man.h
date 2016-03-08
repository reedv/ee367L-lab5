#define MAXHOSTS 100

typedef struct{ /* Connection used by the manager to a host implemented as file descriptors */
   int toHost[2]; 		/* Pipe link to host */
   int fromHost[2]; 	/*  Pipe link from host */
} managerLink;

typedef struct {
   int numlinks;
   managerLink links[MAXHOSTS];  /* array of links to host that manager can access*/
} manLinkArrayType;

/* 
 * Main loop for the manager.  It repeatedly gets command from
 * the user and then executes the command
 */
void managerMain(manLinkArrayType * manLinkArray);


