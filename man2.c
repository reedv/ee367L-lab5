
void manDisplayHosts(int currhost, int maxHosts) 
{
int i; 

printf("List of hosts:\n");

for (i=0; i<maxHosts; i++) {
   if (i == currhost) printf("   Host ID = %d (connected)\n",i);
   else printf("   Host ID = %d\n",i);
}
printf("\n");
}

int manChangeHost(int maxHosts) 
{
int newnumber;

do {
   printf("Enter host ID number (range 0 to %d): ", maxHosts-1);
   scanf("%d",&newnumber);
   if (newnumber >= 0 && newnumber < maxHosts) break;
   else printf("Number is out of range, try again\n\n");
} while(1);

return newnumber;
}



/***************************** 
 * Main loop of the manager  *
 *****************************/
void managerMain(manLinkArrayType * manLinkArray)
{
int currhost;      /* The current host the manager is connected to */
char cmd;          /* Command entered by user */
int k;

currhost = 0;      /* Manager is initially connected to host 0 */

while(1) {
   cmd = manGetUserCommand(currhost);
   if (cmd == 'q') return;
   else if (cmd == 'd') {
      manGetHostState(&(manLinkArray->links[currhost]));
      manWaitForReply(&(manLinkArray->links[currhost]), cmd);
   } 
   else if (cmd == 's') {
      manSetNetAddr(&(manLinkArray->links[currhost]));
      manWaitForReply(&(manLinkArray->links[currhost]), cmd);
   } 
   else if (cmd == 'm') { 
      manSetMainDir(&(manLinkArray->links[currhost]));
      manWaitForReply(&(manLinkArray->links[currhost]), cmd);
   }
   else if (cmd == 'f') {
      manClearRcvFlg(&(manLinkArray->links[currhost]));
      manWaitForReply(&(manLinkArray->links[currhost]), cmd);
   }
   else if (cmd == 'r') {
      manDownloadPacket(&(manLinkArray->links[currhost])); 
      manWaitForReply(&(manLinkArray->links[currhost]), cmd);
   }
   else if (cmd == 'u') {
      manUploadPacket(&(manLinkArray->links[currhost])); 
      manWaitForReply(&(manLinkArray->links[currhost]), cmd);
   }
   else if (cmd == 't') {
      k = manTransmitPacket(&(manLinkArray->links[currhost]));
      if (k==0) manWaitForReply(&(manLinkArray->links[currhost]), cmd);
   }
   else if (cmd == 'h') 
      manDisplayHosts(currhost, manLinkArray->numlinks);

   else if (cmd == 'c') 
      currhost = manChangeHost(manLinkArray->numlinks);

   else printf("***Invalid command, you entered %c\n", cmd);
}
printf("\n");
  
} 


