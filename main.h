
#define PAYLOAD_LENGTH 200 /* Maximum payload size */

typedef struct { /* Packet buffer */
   int src_addr;  /* Source address */
   int dest_addr;  /* Destination addres */
   int length;   /* Length of packet */
   char payload[PAYLOAD_LENGTH + 1];  /* Payload section */
   int is_valid;   /* Indicates if the contents is valid */ 
   int new;     /* Indicates if the contents has been downloaded */
} packetBuffer;

   


