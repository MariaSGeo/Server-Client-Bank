#include "bankaccounts.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

using namespace std;


void swapi(int *a, int *b){ 
	int temp;
	temp = *a;
	*a = *b;
	*b = temp; 
}

void orderkeys(int *keys,int n){ 
	int i, j;
	for (i=1 ; i <= n-1 ; i++) { 
	j = i-1;
		while (j >= 0 && keys[j] > keys[j+1]) {
			swapi(&keys[j], &keys[j+1]); 
			j--; 
		} 
	} 
}


void uniques(int *keys,int n){
	
	int current=keys[0];
	int * newkeys = new int [n];
	int counter=1;
	newkeys[0]=keys[0];
	for(int i=1;i<n;i++)
		newkeys[i]=-1;

	for(int i=1;i<n;i++){
		if(current!=keys[i]){
			newkeys[counter]=keys[i];
			current=keys[i];
			counter ++;
		}
	}
	for(int i=0;i<n;i++)
		swapi(&keys[i],&newkeys[i]);

	delete newkeys;		
}



//-------------------------------------------------TRANSFER-----------------------
transfer::transfer(char * n,int am){

	account_name = (char *)malloc(strlen(n));
	strncpy(account_name,n,strlen(n));
	amount=am;
	next=NULL;

}


transfer::~transfer(){free(account_name);}
		
char * transfer::get_account_name(){return account_name;}

int transfer::transfer_get_amount(){return amount;}
		

void transfer::set_next(transfer * n){next=n;}

transfer *  transfer::get_next(){return next;}

void transfer::transfer_update_amount(int am){amount+=am;}

//-------------------------------------------------ACCOUNT-----------------------

account::account(char * n,int am){

	amount=am;
	name = (char *)malloc(strlen(n));
	strncpy(name,n,strlen(n));
	numof_transfers=0;
	next=NULL;
	first=NULL;

}


account::~account(){
	transfer * tmp=first;
	while(tmp!=NULL){
		transfer * tmp2 = tmp->get_next();
		delete tmp;
		tmp=tmp2;
	}
	free(name);
}

account *  account::get_next(){return next;}

transfer *  account::get_first(){return first;}

int account::numofmsgs(){return numof_transfers;}

void account::set_next(account * na){next=na;};

char * account::get_account_name(){return name;}

int account::get_account_amount(){return amount;}

void account::account_update(int am){amount-=am;}

void account::insert_new_transfer(char * source,int am){ 	//insert at start

	transfer * newt = new transfer(source,am);

	if(first==NULL)
		first = newt;
	else{

		newt->set_next(first);
		first = newt;		
	}

	numof_transfers++;
}

void account::account_add_transfer(int am,char * source){

	transfer * tmp = first;
	
	while(tmp!=NULL){
	
		if (strcmp(source,tmp->get_account_name())==0)
			tmp->transfer_update_amount(am);
		else	
			insert_new_transfer(source,am);
		tmp=tmp->get_next();
	}

	amount+=am;	
};

//-------------------------------------------BANK_ACCOUNTS-----------------------------


bank_accounts::bank_accounts(int s){
	
	size=s;
	buckets = (account **)malloc(sizeof(account *)*s);
	bucketmutexes = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t)*s);
	for(int i=0;i<size;i++){
		buckets[i] = NULL;
		pthread_mutex_init(&bucketmutexes[i],NULL);
	
	}

}

void bank_accounts::delete_bucket_content(int i){ //delete each buvkets content

	account * start = buckets[i];
	account * tmp = start;
	while(start !=NULL ){
		tmp=start->get_next();
		delete start;
		start=tmp;
	}

}

bank_accounts::~bank_accounts(){

	for(int i=0;i<size;i++){ 
		delete_bucket_content(i);
		pthread_mutex_destroy(&bucketmutexes[i]); 
	}
	free (buckets);
	free (bucketmutexes);

}


account * bank_accounts::get_account(char * name){

	int key = hash_func(name);
	pthread_mutex_lock(&bucketmutexes[key]);
	account * current=buckets[key];
	while(current!=NULL){
	 	char * current_name = current->get_account_name();
 	
	 	if(strcmp(current_name,name)==0){

			pthread_mutex_unlock(&bucketmutexes[key]);

	 		return current;
	 	}
	 	current=current->get_next();
	}
	pthread_mutex_unlock(&bucketmutexes[key]);
	return NULL;
}

int bank_accounts::get_size(){return size;}

int bank_accounts::hash_func(const char* s){
	
    unsigned int hash = 0;
    while (*s)
    {
        hash = hash * 101  +  *s++;
    }
    if(hash<0)
    	hash=-hash;

    return hash%size;
}

bool bank_accounts::add_account(char * name,int amount,int delay){
	
	account * n=new account(name,amount);

	if(get_account(name)==NULL){
		account * insert_here;
		int key = hash_func(name);
		pthread_mutex_lock(&bucketmutexes[key]);
		if(delay>0)
			usleep(delay);

		insert_here = buckets[key];
		if(buckets[key]==NULL){
			buckets[key] = n;
		}else{
			n->set_next(buckets[key]);
			buckets[key] = n;
		}
		pthread_mutex_unlock(&bucketmutexes[key]);

		cout <<"INSERTED "<<name << " "<<key<<endl;
		return true;
	}else{
		int key = hash_func(name);
		pthread_mutex_lock(&bucketmutexes[key]);
		if(delay>0)
			usleep(delay);
		delete n;
		pthread_mutex_unlock(&bucketmutexes[key]);

		return false;
	}
}

bool bank_accounts::add_transfer(int amount,char * source,char * dest,int delay){

	

	account * sourceacc = get_account(source);
	account * destacc = get_account(dest);
	int cases=0;
	int key1 = hash_func(source);
	int key2 = hash_func(dest);
	if(key1==key2){
		pthread_mutex_lock(&bucketmutexes[key1]);

	}else if(key1>key2){
		pthread_mutex_lock(&bucketmutexes[key2]);
		pthread_mutex_lock(&bucketmutexes[key1]);
		cases=1;


	}else{
		pthread_mutex_lock(&bucketmutexes[key1]);
		pthread_mutex_lock(&bucketmutexes[key2]);

		cases=2;

	}
	if(delay>0)
		usleep(delay);

	if((sourceacc!=NULL)&&(destacc!=NULL)) {
		
		if(sourceacc->get_account_amount()>=amount){

			sourceacc->account_update(amount);
			destacc->account_add_transfer(amount,source);
			if(cases == 0){
				pthread_mutex_unlock(&bucketmutexes[key1]);
			}else if(cases==1){
				pthread_mutex_unlock(&bucketmutexes[key2]);
				pthread_mutex_unlock(&bucketmutexes[key1]);

			}else if(cases==2){
				pthread_mutex_unlock(&bucketmutexes[key1]);
				pthread_mutex_unlock(&bucketmutexes[key2]);

			}

			return true;
		}

		if(cases == 0){
			pthread_mutex_unlock(&bucketmutexes[key1]);

		}else if(cases==1){
			pthread_mutex_unlock(&bucketmutexes[key2]);
			pthread_mutex_unlock(&bucketmutexes[key1]);

		}else if(cases==2){
			pthread_mutex_unlock(&bucketmutexes[key1]);
			pthread_mutex_unlock(&bucketmutexes[key2]);
		}

		return false;	
	}else{
		
		if(cases == 0){
			pthread_mutex_unlock(&bucketmutexes[key1]);

		}else if(cases==1){
			pthread_mutex_unlock(&bucketmutexes[key2]);
			pthread_mutex_unlock(&bucketmutexes[key1]);

		}else if(cases==2){
			pthread_mutex_unlock(&bucketmutexes[key1]);
			pthread_mutex_unlock(&bucketmutexes[key2]);
		}
		
		return false;

	}

}

bool bank_accounts::add_multi_transfer(int amount,char * source,char ** dests,int destsize,int delay){

	

	account * sourceacc = get_account(source);

	account ** destaccs = new account*[destsize];
	int * keys = new int[destsize];
	int sourcekey = hash_func(source);
	for(int i=0;i<destsize;i++){
		keys[i] = hash_func(dests[i]);
		destaccs[i] = get_account(dests[i]);

	}
	
	orderkeys(keys,destsize);
	uniques(keys,destsize);
	pthread_mutex_lock(&bucketmutexes[sourcekey]);
	for(int i=0;i<destsize;i++){
		if((keys[i]>-1)&&(keys[i]!=sourcekey))
			pthread_mutex_lock(&bucketmutexes[keys[i]]);
	}
	if(delay>0)
		usleep(delay);


	for(int i=0;i<destsize;i++){
		
		if(destaccs[i]==NULL){
			delete destaccs;
			pthread_mutex_unlock(&bucketmutexes[sourcekey]);
			for(int i=0;i<destsize;i++){
				if((keys[i]>-1)&&(keys[i]!=sourcekey))
					pthread_mutex_unlock(&bucketmutexes[keys[i]]);
			}
			return false;
		}
	}
	
	if(sourceacc==NULL){
		pthread_mutex_unlock(&bucketmutexes[sourcekey]);
		for(int i=0;i<destsize;i++){
			if((keys[i]>-1)&&(keys[i]!=sourcekey))
				pthread_mutex_unlock(&bucketmutexes[keys[i]]);
		}
		return false;
	}


	if(sourceacc->get_account_amount()>=(amount*destsize)){
		
		for(int i=0;i<destsize;i++){
			
			sourceacc->account_update(amount);
			destaccs[i]->account_add_transfer(amount,source);
			
		}

		pthread_mutex_unlock(&bucketmutexes[sourcekey]);
		for(int i=0;i<destsize;i++){
			if((keys[i]>-1)&&(keys[i]!=sourcekey))
				pthread_mutex_unlock(&bucketmutexes[keys[i]]);
		}
		return true;
	}
	pthread_mutex_unlock(&bucketmutexes[sourcekey]);	
	for(int i=0;i<destsize;i++){
		if((keys[i]>-1)&&(keys[i]!=sourcekey))
			pthread_mutex_unlock(&bucketmutexes[keys[i]]);
	}
	return false;
}

int bank_accounts::print_balance(char * name,int delay){

	account * acc = get_account(name);
	int key = hash_func(name);
	pthread_mutex_lock(&bucketmutexes[key]);
	if(delay>0)
		usleep(delay);
	if(acc==NULL){
		pthread_mutex_unlock(&bucketmutexes[key]);
		return -1;
	

	}else{
		
		int am = acc->get_account_amount();
		pthread_mutex_unlock(&bucketmutexes[key]);

		return	am;
	}

}

int * bank_accounts::print_multi_balance(char ** name,int s,int delay){

	int * amounts = (int *)malloc(sizeof(int)*s);
	account ** accounts = new account*[s];

	int * keys = new int[s];

	for(int i=0;i<s;i++){
		
		accounts[i]=get_account(name[i]);
		keys[i] = hash_func(name[i]);
	}
	orderkeys(keys,s);
	uniques(keys,s);
	for(int i=0;i<s;i++){
		if(keys[i]>-1)
		pthread_mutex_lock(&bucketmutexes[keys[i]]);		
	}
	if(delay>0)
		usleep(delay);
	for(int i=0;i<s;i++){
		if(accounts[i]==NULL){
			delete keys;
			delete accounts;
			for(int i=0;i<s;i++){
				if(keys[i]>-1)
				pthread_mutex_unlock(&bucketmutexes[keys[i]]);		
			}
			return NULL;
		}
	}
	
	for(int i=0;i<s;i++){
		
		amounts[i] = accounts[i]->get_account_amount();
	}
	for(int i=0;i<s;i++){
		if(keys[i]>-1)
			pthread_mutex_unlock(&bucketmutexes[keys[i]]);		
	}
	
	return amounts;

}