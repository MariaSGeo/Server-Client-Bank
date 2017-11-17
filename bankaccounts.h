#ifndef BANKACCOUNTS_H_
#define BANKACCOUNTS_H_
#include <pthread.h>


//-----------------------------------------TRANSFER---------------------------------

class transfer{   

	char * account_name;
	transfer * next;
	int amount;

	public:
		transfer(char * m,int f);
		~transfer();
		int transfer_get_amount();
		void transfer_update_amount(int am);
		void set_next(transfer * n);
		char * get_account_name();
		transfer * get_next();
	
};

//-----------------------------------------ACCOUNT---------------------------------

class account{	

	char * name;
	transfer * first;
	int amount;
	account * next;
	int numof_transfers;

	public:

		account(char * n,int am);
		~account();
		account * get_next();
		transfer * get_first();
		char * get_account_name();
		int get_account_amount();		
		void set_next(account * nl);
		void insert_new_transfer(char * source,int am); //insert at start
		int numofmsgs();
		void account_add_transfer(int amount,char * source);
		void account_update(int am);
};

//-----------------------------------------ACCOUNTS---------------------------------


class bank_accounts  
{

	int size;
	account ** buckets;
	pthread_mutex_t * bucketmutexes;
		
	void delete_bucket_content(int b);

	public:
		bank_accounts(int s);
		~bank_accounts();
		void new_account(char * name);
		account * get_account(char * name);
		int get_size();
		int hash_func(const char* s);		
		bool add_account(char * name,int amount,int delay);
		bool add_transfer(int amount,char * source,char * dest,int delay);
		bool add_multi_transfer(int amount,char * source,char ** dests,int dsize,int delay);
		int print_balance(char * name,int delay);
		int * print_multi_balance(char ** name,int s,int delay);
			
};

#endif