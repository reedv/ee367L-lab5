/* Definitions and prototypes for the link (link.c)
 */

#define MAXLINKS 100

enum LinkType {UNIPIPE}; /* UNIPIPE = unidirectional pipe
                          * We can add more types later
                          */

enum UniPipeType {BLOCKING, NONBLOCKING};

typedef struct {
   enum UniPipeType pipeType;
   int         pipe_filedes[2];
   int         src_physId;
   int	       dest_physId;
} UniPipeInfo;

typedef struct {  /* Has all the information to implement a link */
   int linkID;             /* ID for the link */
   enum LinkType linkType; /* The type of link */
   UniPipeInfo uniPipeInfo; /* If the link is a pipe, this is the information */
} LinkInfo;


typedef struct {
   int numlinks;
   LinkInfo link[MAXLINKS];
} linkArrayType;

/* Transmit the packet in pbuff on the link */
int linkSend(LinkInfo * link, packetBuffer * pbuff);

/* Downloads a packet from the link into pbuff */
int linkReceive(LinkInfo * link, packetBuffer * pbuff);

/* Closes a link */
int linkClear(LinkInfo * link);

/* Initializes a link */
int linkCreate(LinkInfo * link);

// display info of links in link_array for debugging purposes
void linkDisplay(linkArrayType * link_array);

