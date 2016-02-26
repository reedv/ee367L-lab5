/* 
 * host.h 
 */

#define NAME_LENGTH 100 

typedef struct { /* State of host */
   int   physid;              /* physical id */
   char  main_dir[NAME_LENGTH]; /* main directory name */
   int   main_dir_valid;        /* indicates if the main directory is empty */
   int   netaddr;             /* host's network address */
   int   nbraddr;             /* network address of neighbor */
   packetBuffer sendPacketBuff;  /* send packet buffer */
   packetBuffer rcvPacketBuff;   
   managerLink manLink;       /* Connection to the manager */
   LinkInfo link_in;           /* Incoming communication link */
   LinkInfo link_out;          /* Outgoing communication link */
} hostState;

void hostMain(hostState * hstate);

void hostInit(hostState * hstate, int physid);

