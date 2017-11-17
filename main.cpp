#include <stdio.h>    // from www.mario-konrad.ch
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

# include <sys/socket.h>
# include <sys/types.h>
#include <netinet/in.h>                         /* For Internet sockets */
#include <netdb.h>                                 /* For gethostbyaddr */
#include "bankaccounts.h"
#include "bankapp.h"
using namespace std;
//bank_accounts * bank;

#define POOL_SIZE 50
#define BUFFERSIZE 4096



int main(int argc, char ** argv)
{
	
	int port, sock, newsock; char buf[256],queuesize,poolsize;
   	unsigned int serverlen, clientlen;
   	struct sockaddr_in server, client;
   	struct sockaddr *serverptr, *clientptr;
   	struct hostent *rem;
    //connection_info * args;

   	if (argc != 7) {printf("Please give correct number of parameters\n");exit(1);}
    
    if ((strcmp(argv[2],"-q")==0)||(strcmp(argv[4],"-q")==0)||(strcmp(argv[6],"-q")==0)){printf("Flags cannot be in these positions (-h)\n");exit(1);}
    
    if ((strcmp(argv[2],"-s")==0)||(strcmp(argv[4],"-s")==0)||(strcmp(argv[6],"-s")==0)){printf("Flags cannot be in these positions (-i)\n");exit(1);}

    if ((strcmp(argv[2],"-p")==0)||(strcmp(argv[4],"-p")==0)||(strcmp(argv[6],"-p")==0)){printf("Flags cannot be in these positions (-p)\n");exit(1);}

    for(int j=0;j<argc;j++)
        cout << argv[j]<<endl;

    if(strcmp(argv[1],"-q")==0)
        if(strcmp(argv[3],"-s")==0){
            if(strcmp(argv[5],"-p")==0){

                port = atoi(argv[6]);
                poolsize = atoi(argv[4]);
                queuesize = atoi(argv[2]);

            }
        }else{

                port = atoi(argv[4]);
                poolsize = atoi(argv[6]);
                queuesize = atoi(argv[2]);

        }

    if(strcmp(argv[1],"-p")==0)
        if(strcmp(argv[3],"-q")==0){
            if(strcmp(argv[5],"-s")==0){

                port = atoi(argv[2]);
                poolsize = atoi(argv[6]);
                queuesize = atoi(argv[4]);

            }
        }else{

                port = atoi(argv[2]);
                poolsize = atoi(argv[4]);
                queuesize = atoi(argv[6]);
                cout << atoi(argv[2]) <<" "<<atoi(argv[4])<<" "<<atoi(argv[6])<<endl;

        } 


    if(strcmp(argv[1],"-s")==0)
        if(strcmp(argv[3],"-p")==0){
            if(strcmp(argv[5],"-q")==0){

                port = atoi(argv[4]);
                poolsize = atoi(argv[2]);
                queuesize = atoi(argv[6]);


            }
        }else{

                port = atoi(argv[4]);
                poolsize = atoi(argv[6]);
                queuesize = atoi(argv[2]);

        }             
    
        bankapp(poolsize,queuesize,port);

}
