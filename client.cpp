/* inet_str_client.c: Internet stream sockets client */
#include <stdio.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <unistd.h>          /* read, write, close */
#include <netdb.h>	         /* gethostbyaddr */
#include <stdlib.h>	         /* exit */
#include <string.h>	         /* strlen */
#include <iostream>
#include <string.h>

#define BUFFERSIZE 4096
using namespace std;

void perror_exit(const char *message);

int  main(int argc, char *argv[]) {
    int port, sock, i;
    char buf[BUFFERSIZE];
    char temp[BUFFERSIZE];
    char ansbuf[BUFFERSIZE];
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;
    FILE * commandfile;    


    if (argc != 7) {printf("Please give correct number of parameters\n");exit(1);}
    
    //-h <server host> -p <server port> -i <command file>

    if ((strcmp(argv[2],"-h")==0)||(strcmp(argv[4],"-h")==0)||(strcmp(argv[6],"-h")==0)){printf("Flags cannot be in these positions (-h)\n");exit(1);}
    
    if ((strcmp(argv[2],"-i")==0)||(strcmp(argv[4],"-i")==0)||(strcmp(argv[6],"-i")==0)){printf("Flags cannot be in these positions (-i)\n");exit(1);}

    if ((strcmp(argv[2],"-p")==0)||(strcmp(argv[4],"-p")==0)||(strcmp(argv[6],"-p")==0)){printf("Flags cannot be in these positions (-p)\n");exit(1);}



    if(strcmp(argv[1],"-h")==0)
        if(strcmp(argv[3],"-p")==0){
            if(strcmp(argv[5],"-i")==0){

                commandfile = fopen(argv[6], "r");
                port = atoi(argv[4]);
                if ((rem = gethostbyname(argv[2])) == NULL) {herror("gethostbyname"); exit(1);}

            }

        }else{

                commandfile = fopen(argv[4], "r");

                port = atoi(argv[6]);
                if ((rem = gethostbyname(argv[2])) == NULL) {herror("gethostbyname"); exit(1);}
        }

    if(strcmp(argv[1],"-p")==0)
        if(strcmp(argv[3],"-h")==0){
            if(strcmp(argv[5],"-i")==0){

                commandfile = fopen(argv[6], "r");
                port = atoi(argv[2]);
                if ((rem = gethostbyname(argv[4])) == NULL) {herror("gethostbyname"); exit(1);}

            }
        }else{

                commandfile = fopen(argv[4], "r");
                port = atoi(argv[2]);
                if ((rem = gethostbyname(argv[6])) == NULL) {herror("gethostbyname"); exit(1);}
        } 


    if(strcmp(argv[1],"-i")==0)
        if(strcmp(argv[3],"-p")==0){
            if(strcmp(argv[5],"-h")==0){

                commandfile = fopen(argv[2], "r");
                port = atoi(argv[4]);
                if ((rem = gethostbyname(argv[6])) == NULL) {herror("gethostbyname"); exit(1);}

            }
        }else{

                commandfile = fopen(argv[2], "r");
                port = atoi(argv[6]);
                if ((rem = gethostbyname(argv[4])) == NULL) {herror("gethostbyname"); exit(1);}
        }             

    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){perror_exit("socket");}
    
    
    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);         /* Server port */
    
    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0){perror_exit("connect");}

    printf("Connecting to %s port %d\n", argv[1], port);
    
        

   cout<<"READY"<<endl;

    if(commandfile == NULL) {perror_exit("Error opening file");}
    
    int instrcountr=1;
    
    do{
        memset(buf,0,BUFFERSIZE);
        memset(ansbuf,0,BUFFERSIZE);

        if(fgets(buf, BUFFERSIZE, commandfile)==NULL) 
            break;
        strncpy(temp,buf,BUFFERSIZE);
        char * token = strtok(temp," \n");

        if(strcmp(token , "sleep")==0){
            token = strtok(NULL," \n");
            usleep(atoi(token));
            instrcountr++;
            continue;
        }
        if(strcmp(token ,"exit")==0)
            break;

        if (write(sock, buf, BUFFERSIZE) < 0){perror_exit("write");}
        

        if (read(sock, ansbuf, BUFFERSIZE) < 0){perror_exit("read");}   
        
        cout <<instrcountr <<" "<< ansbuf<<endl;
        instrcountr++;
    }while (strcmp(buf, "exit\n") != 0); 
    
    fclose(commandfile);

    do{
        memset(buf,0,BUFFERSIZE);
        memset(ansbuf,0,BUFFERSIZE);

    	cout << "Give new command : " <<endl;
    	if(fgets(buf, BUFFERSIZE, stdin)==NULL) 
            break;	
        strncpy(temp,buf,BUFFERSIZE);
        char * token = strtok(temp," \n");
        if(strcmp(token , "sleep")==0){
            token = strtok(NULL," \n");
            usleep(atoi(token));
            instrcountr++;
            continue;
        }
        if(strcmp(token ,"exit")==0)
            break;
        if(strcmp(token ,"exit\n")==0)
            break;
        if (write(sock, buf, BUFFERSIZE) < 0){perror_exit("write");}
        

        if (read(sock, ansbuf, BUFFERSIZE) < 0){perror_exit("read");}   
        cout <<instrcountr <<" "<< ansbuf<<endl;
        instrcountr++;
    }while (strcmp(buf, "exit\n") != 0); 


    close(sock);                 
    return 1;
}			     

void perror_exit(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
