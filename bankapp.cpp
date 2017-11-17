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
bank_accounts * bank;

#define POOL_SIZE 50
#define BUFFERSIZE 4096

void perror_exit(const char *message)
{
    perror(message);
    cout<<EXIT_FAILURE<<endl;
    exit(EXIT_FAILURE);
}
typedef struct{
    
    int socket_fd;
    unsigned int serverlen;
    struct sockaddr *serverptr;     

}connection_info;


typedef struct {
	int * data;
	int start;
	int end;
	int count;
    int maxsize;
} pool_t;

int num_of_items = 15;

pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;//there is a request
pthread_cond_t cond_nonfull;//the request queue is full
pool_t pool;

void initialize(pool_t * pool,int qsize) {
	
    pool->data = (int *)malloc(sizeof(int)* qsize);
    pool->maxsize = qsize;
    pool->start = 0;
	pool->end = -1;
	pool->count = 0;
}

void place(pool_t * pool, int data) {
	pthread_mutex_lock(&mtx);
	while (pool->count >= POOL_SIZE) {
		printf(">> Found Buffer Full \n");
		pthread_cond_wait(&cond_nonfull, &mtx);
		}

    pool->end = (pool->end + 1) % POOL_SIZE;
    pool->data[pool->end] = data;
    pool->count++;
    cout<<"placed "<<data<<" position "<<pool->end<<endl;
    pthread_mutex_unlock(&mtx);
}

int obtain(pool_t * pool) {
	int data = 0;
	pthread_mutex_lock(&mtx);
	while (pool->count <= 0) {
		printf(">> Found Buffer Empty \n");
		pthread_cond_wait(&cond_nonempty, &mtx);
		}
	data = pool->data[pool->start];
    cout << "Got  "<<data <<" from pos "<<pool->start;
	pool->start = (pool->start + 1) % POOL_SIZE;
	pool->count--;
	pthread_mutex_unlock(&mtx);
	return data;
}


void * consumer(void * ptr){
	

   while(1){

        int request = obtain(&pool);
        int nwrite;
        printf("consumer: %d\n", request);
        char buf[BUFFERSIZE];
        char tmpbuf[BUFFERSIZE];
        char ansbuf[BUFFERSIZE];
  		pthread_cond_signal(&cond_nonfull);
        cout<<"waiting to read "<<endl;

        while(read(request, buf, BUFFERSIZE) > 0) {
            
            memset(ansbuf,0,BUFFERSIZE);
            
            printf("connection : %d\n", request);
            strncpy(tmpbuf,buf,BUFFERSIZE);
            cout <<buf<<endl;

            char * strtokmsg;
            char * token=strtok_r(tmpbuf," \n",&strtokmsg);
            if (token==NULL){
                memset(buf,0,BUFFERSIZE);
                memset(ansbuf,0,BUFFERSIZE);
                strcpy(ansbuf,"Server : Not valid instruction1");
                if (write(request, ansbuf, BUFFERSIZE) < 0){perror_exit("write");}
                continue;
            }

            if(strcmp(token , "add_account")==0){
                int amount,delay;
                if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                    memset(ansbuf,0,BUFFERSIZE);
                    strcpy(ansbuf,"Server : Not valid instruction2");
                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                    memset(buf,0,BUFFERSIZE);
                    continue;
                }else{
                    amount = atoi(token);
                    if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                    
                        memset(ansbuf,0,BUFFERSIZE);
                        strcpy(ansbuf,"Server : Not valid instruction3");
                        if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                        memset(buf,0,BUFFERSIZE);
                        continue;

                    }else{  
                        char name[80];                        
                        strcpy(name,token);
                        if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                            delay=0;
                            bool result=bank->add_account(name,amount,delay);
                            memset(ansbuf,0,BUFFERSIZE);

                            if(!result)   
                                strcpy(ansbuf,"Server : add_account failed ");
                            else
                                strcpy(ansbuf,"Server : add_account success ");

                            strcat(ansbuf," ");
                            strcat(ansbuf,name);
                            strcat(ansbuf,":");
                            char camount[10];
                            sprintf(camount,"%d",amount);

                            strcat(ansbuf,camount);
                            if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1) { perror("Error in Writing"); exit(2); }
                            
                            memset(buf,0,BUFFERSIZE);
                            continue;

                        }else{
                            delay=atoi(token);
                            if((token = strtok_r(NULL," \n",&strtokmsg))!=NULL ) {
                                memset(ansbuf,0,BUFFERSIZE);
                                strcpy(ansbuf,"Server : Not valid instruction4");
                                if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                            }else{

                                bool result=bank->add_account(name,amount,delay);
                                memset(ansbuf,0,BUFFERSIZE);

                                if(!result)    
                                    strcpy(ansbuf,"Server : add_account failed ");
                                else
                                    strcpy(ansbuf,"Server : add_account success ");
                                strcat(ansbuf," ");
                                strcat(ansbuf,name);
                                strcat(ansbuf,":");
                                char camount[10];
                                sprintf(camount,"%d",amount);
                                strcat(ansbuf,camount);
                                strcat(ansbuf," [ ");
                                char cdel[10];
                                sprintf(cdel,"%d",delay);
                                strcat(ansbuf,cdel);
                                strcat(ansbuf," ] ");
                                if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1) { perror("Error in Writing"); exit(2); }
                        
                                memset(buf,0,BUFFERSIZE);
                                continue;

                            }
                        }
                        
                    }
                }
            }
            if(strcmp(token , "add_transfer")==0) {
                int amount,delay;
                if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                    memset(ansbuf,0,BUFFERSIZE);
                    strcpy(ansbuf,"Server : Not valid instruction2");
                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                    memset(buf,0,BUFFERSIZE);
                    continue;

                }else{
                    amount = atoi(token);
                    if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                        memset(ansbuf,0,BUFFERSIZE);
                        strcpy(ansbuf,"Server : Not valid instruction3");
                        if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                            memset(buf,0,BUFFERSIZE);
                            continue;

                    }else{  
                        char name1[80];                       
                        strcpy(name1,token);
                        if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                            memset(ansbuf,0,BUFFERSIZE);
                            strcpy(ansbuf,"Server : Not valid instruction3");
                            if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                            memset(buf,0,BUFFERSIZE);
                            continue;

                        }else{
                                
                                char name2[80];
                               
                                strcpy(name2,token);

                                if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                                    delay=0;
                                    //cout <<"DEL 0"<<endl;
                                    bool result=bank->add_transfer(amount,name1,name2,delay);
                                    memset(ansbuf,0,BUFFERSIZE);
 
                                    if(!result)    
                                        strcpy(ansbuf,"Server : add_transfer failed ");
                                    else
                                        strcpy(ansbuf,"Server : add_transfer success ");
                                    
                                    strcat(ansbuf," ");                                    
                                    char camount[10];
                                    sprintf(camount,"%d",amount);
                                    strcat(ansbuf,camount);
                                    strcat(ansbuf," ");
                                    strcat(ansbuf,name1);
                                    strcat(ansbuf," ");
                                    strcat(ansbuf,name2);
                                   
                                    
                                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1) { perror("Error in Writing"); exit(2); }
                    
                                    memset(buf,0,BUFFERSIZE);
                                    continue;

                                }else{
                                    delay=atoi(token);
                                    if((token = strtok_r(NULL," \n",&strtokmsg))!=NULL ) {
                                        memset(ansbuf,0,BUFFERSIZE);
                                        strcpy(ansbuf,"Server : Not valid instruction4");
                                        
                                        if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                                        memset(buf,0,BUFFERSIZE);
                                        continue;

                                    }else{

                                        bool result=bank->add_transfer(amount,name1,name2,delay); 
                                        memset(ansbuf,0,BUFFERSIZE);
                                        if(!result)    
                                            strcpy(ansbuf,"Server : add_transfer failed ");
                                        else
                                            strcpy(ansbuf,"Server : add_transfer success ");
                                        strcat(ansbuf," ");
                                        strcat(ansbuf,name1);
                                        strcat(ansbuf," ");
                                        strcat(ansbuf,name2);
                                       
                                        char camount[10];
                                        sprintf(camount,"%d",amount);
                                        strcat(ansbuf,camount);
                                        strcat(ansbuf," [ ");
                                        char cdel[10];
                                        sprintf(cdel,"%d",delay);
                                        strcat(ansbuf,cdel);
                                        strcat(ansbuf," ] ");
                                        if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1) { perror("Error in Writing"); exit(2); }
                                       
                                        memset(buf,0,BUFFERSIZE);
                                        continue;


                                    }
                                }
                            }            
                        
                    }
                }
            }

            if(strcmp(token , "print_balance")==0){
                if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                    memset(ansbuf,0,BUFFERSIZE);
                    strcpy(ansbuf,"Server : Not valid instruction3");
                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                    memset(buf,0,BUFFERSIZE);
                    continue;
                }else{

                    char name[80];
                    strcpy(name,token);
                    int delay;
                    if((token = strtok_r(NULL," \n",&strtokmsg))!=NULL ) {
                        delay=atoi(token);
                        if((token = strtok_r(NULL," \n",&strtokmsg))!=NULL ) {    
                            memset(ansbuf,0,BUFFERSIZE);                        
                            strcpy(ansbuf,"Server : Not valid instruction4");
                            //free(name);
                            if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                            memset(buf,0,BUFFERSIZE);
                            continue;
                        }
                        int result=bank->print_balance(name,delay);
                            if(result==-1)    
                                strcpy(ansbuf,"Server : print_balance failed ");
                            else
                                strcpy(ansbuf,"Server : print_balance success ");
                            strcat(ansbuf," ");
                            strcat(ansbuf,name);
                            if(result>-1){  
                                strcat(ansbuf," : ");
                                char camount[10];
                                sprintf(camount,"%d",result);
                                strcat(ansbuf,camount);
                                strcat(ansbuf,"[");
                                char damount[10];
                                sprintf(damount,"%d",delay);
                                strcat(ansbuf,damount);     
                                strcat(ansbuf,"]");

                            }
                            if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                            memset(buf,0,BUFFERSIZE);
                            continue;
                        

                    }else{
                        delay=0;
                        memset(ansbuf,0,BUFFERSIZE);

                            int result=bank->print_balance(name,delay);
                            if(result==-1)    
                                strcpy(ansbuf,"Server : print_balance failed ");
                            else
                                strcpy(ansbuf,"Server : print_balance success ");
                            strcat(ansbuf," ");
                            strcat(ansbuf,name);
                            if(result>-1){  
                                strcat(ansbuf," : ");
                                char camount[10];
                                sprintf(camount,"%d",result);
                                strcat(ansbuf,camount);
                            }
                            if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                            memset(buf,0,BUFFERSIZE);
                            continue;
                    }
                }

            }


            if(strcmp(token , "print_multi_balance")==0){
                   if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                    memset(ansbuf,0,BUFFERSIZE);
                    strcpy(ansbuf,"Server : Not valid instruction3");
                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                    memset(buf,0,BUFFERSIZE);
                    continue; 
                }else{
                   
                    char newb[BUFFERSIZE];
                    char newb2[BUFFERSIZE];  
                    strncpy(newb,buf,BUFFERSIZE);
                    strncpy(newb2,buf,BUFFERSIZE);

                    int counter=0;
                    int maxsize=0;
                    char * token2=strtok_r(newb," \n",&strtokmsg);
                    while((token2 = strtok_r(NULL," \n",&strtokmsg))!=NULL){
                        counter++;
                        if(strlen(token2)>maxsize)
                            maxsize=strlen(token2);
                    }
                    
                    char ** accounts = new char *[counter];
                    for(int i = 0; i <counter; i++)
                        accounts[i] = new char[maxsize];
                    char * token3=strtok_r(newb2," \n",&strtokmsg);
                    int accountnum=0;
                    while((token3 = strtok_r(NULL," \n",&strtokmsg))!=NULL){
                        strcpy(accounts[accountnum],token3);
                        accountnum++;
                    }
                    int delay=0; 
                    int j;
                    for(j=0;j<strlen(accounts[counter-1]);j++)
                        if(isdigit(accounts[counter-1][j])==0) {
                            delay=0;
                            break;
                        }
                       
                    if(j==strlen(accounts[counter-1]))
                        delay = atoi(accounts[counter-1]);
                    int counter2;
                    if(delay>0)
                        counter2=counter-1;
                    else
                        counter2=counter;
                    int * result = bank->print_multi_balance(accounts,counter2,delay);
                    
                    if(result==NULL)    
                        strcpy(ansbuf,"Server1 : print_multi_balance failed ");
                    else
                        strcpy(ansbuf,"Server2 : print_multi_balance success ");
                     
                    for(int i=0;i<counter2;i++){
                        strcat(ansbuf,accounts[i]);
                        if(result!=NULL){
                            strcat(ansbuf,"/");
                            char camount[10];
                            sprintf(camount,"%d",result[i]);
                            strcat(ansbuf,camount);
                            strcat(ansbuf," : ");
                        }else{
                            strcat(ansbuf," : ");

                        }                        
                    }
                    if(counter2==counter-1){
                            strcat(ansbuf,"[");
                            char camount[10];
                            sprintf(camount,"%d",delay);
                            strcat(ansbuf,camount);
                            strcat(ansbuf,"]");
                    }        
                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                    for(int i=0;i<counter;i++)
                        delete  accounts[i];
                    delete accounts;
                    delete result;
                    memset(buf,0,BUFFERSIZE);
                    continue;



                    
                }
            }
            if(strcmp(token , "add_multi_transfer")==0){
                if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                    memset(ansbuf,0,BUFFERSIZE);
                    strcpy(ansbuf,"Server : Not valid instruction3");
                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                    memset(buf,0,BUFFERSIZE);
                    continue; 
                }else{
                    int amount = atoi(token);
                    if((token = strtok_r(NULL," \n",&strtokmsg))==NULL){
                        memset(ansbuf,0,BUFFERSIZE);
                        strcpy(ansbuf,"Server : Not valid instruction3");
                        if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                        memset(buf,0,BUFFERSIZE);
                        continue;
                    }
                    char newb[BUFFERSIZE];
                    char newb2[BUFFERSIZE];  
                    strncpy(newb,buf,BUFFERSIZE);
                    strncpy(newb2,buf,BUFFERSIZE);

                    int counter=0;
                    int maxsize=0;
                    char * token2=strtok_r(newb," \n",&strtokmsg);
                    strtok_r(NULL," \n",&strtokmsg);
                    while((token2 = strtok_r(NULL," \n",&strtokmsg))!=NULL){
                        counter++;
                        
                        if(strlen(token2)>maxsize)
                            maxsize=strlen(token2);
                       
                    }
                             
                    if(counter<2){

                        memset(ansbuf,0,BUFFERSIZE);
                        strcpy(ansbuf,"Server : Not valid instruction3");
                        if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                        memset(buf,0,BUFFERSIZE);
                        continue;
                    }
                    
                    char ** accounts = new char *[counter];
                    for(int i = 0; i <counter; i++)
                        accounts[i] = new char[maxsize];
                    char * token3=strtok_r(newb2," \n",&strtokmsg);
                    strtok_r(NULL," \n",&strtokmsg);

                    int accountnum=0;

                    while((token3 = strtok_r(NULL," \n",&strtokmsg))!=NULL){

                        strcpy(accounts[accountnum],token3);
                        accountnum++;
                    }

                    int delay;
                    int j;
                    for(j=0;j<strlen(accounts[counter-1]);j++)
                        if(isdigit(accounts[counter-1][j])==0) {
                            delay=0;
                            break;
                        }

                    if(j==strlen(accounts[counter-1]))
                        delay = atoi(accounts[counter-1]);
                    int destaccountscounter;
                    if(delay>0)
                        destaccountscounter=counter-2;
                    else
                        destaccountscounter=counter-1;        

                    char ** destaccounts = &accounts[1];

                    bool result = bank->add_multi_transfer(amount,accounts[0],destaccounts,destaccountscounter,delay);
                    
                    if(!result)    
                        strcpy(ansbuf,"Server : add_multi_transfer failed ");
                    else
                        strcpy(ansbuf,"Server : add_multi_transfer success ");
                    
                    strcat(ansbuf,accounts[0]);
                    strcat(ansbuf," ");
                    char camount[10];
                    sprintf(camount,"%d",amount);
                    strcat(ansbuf,camount);
                    strcat(ansbuf," ");

                    for(int i=1;i<counter;i++){
                        if((delay>0)&&(i==counter-1)){
                            strcat(ansbuf,"[");
                            char cdel[10];
                            sprintf(cdel,"%d",delay);
                            strcat(ansbuf,cdel);
                            strcat(ansbuf,"]");
                            break;
                        }

                        strcat(ansbuf,accounts[i]);
                        strcat(ansbuf," ");
                        
                    }
                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                     for(int i=0;i<counter;i++)
                        delete  accounts[i];
                    delete accounts;
                    memset(buf,0,BUFFERSIZE);
                    continue;
                }

            }else{
                
                    memset(ansbuf,0,BUFFERSIZE);
       
                    strcpy(ansbuf,"Server : not valid instruction ");
                    if (nwrite=write(request,ansbuf, BUFFERSIZE) == -1){ perror("Error in Writing"); exit(2); }
                    memset(buf,0,BUFFERSIZE);
                    continue;

            }

            

        }
        if (close(request)==-1){perror_exit("close");}

  		usleep(500000);
    }    		
	pthread_exit(0);
}



void bankapp(int poolsize,int queuesize,int port){

	int sock, newsock;
   	unsigned int serverlen, clientlen;
   	struct sockaddr_in server, client;
   	struct sockaddr *serverptr, *clientptr;
   	struct hostent *rem;
    connection_info * args;

	initialize(&pool,queuesize);
    
    bank=new bank_accounts(50);
    

  	pthread_t * workers = (pthread_t *)malloc(sizeof(pthread_t)*poolsize); //pool of threads
  	for(int i=0;i<poolsize;i++)
  		pthread_create(&workers[i],0,consumer,0);
    

   	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {perror_exit("socket"); }
   
   
   	server.sin_family = PF_INET;                      /* Internet domain */
   	server.sin_addr.s_addr = htonl(INADDR_ANY);   /* My Internet address */
   	server.sin_port = htons(port);                     /* The given port */
   	serverptr = (struct sockaddr *) &server;
   	serverlen = sizeof server;

   	

    args = (connection_info * )malloc(sizeof(connection_info));
    args->socket_fd = sock;
    args->serverlen = serverlen;
    args->serverptr = serverptr;



    if (bind(args->socket_fd, args->serverptr, args->serverlen) < 0) {perror_exit("bind"); }
    
    if (listen(args->socket_fd, 5) < 0) {perror_exit("listen");}

    while(1) {

        clientptr = (struct sockaddr *) &client;
        clientlen = sizeof client;

        if ((newsock = accept(args->socket_fd, clientptr, &clientlen)) < 0){perror_exit("accept"); }              

       
        if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr,sizeof client.sin_addr.s_addr,client.sin_family)) == NULL){perror_exit("gethostbyaddr");  }
        
        cout<<"Accepted connection from "<< rem -> h_name<<endl;
        place(&pool, newsock);
        cout << "Placed descriptor "<<newsock<<" in list " << endl;
       
        pthread_cond_signal(&cond_nonempty); //there is a request
        usleep(0);
    }         
}