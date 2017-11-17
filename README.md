# Bank Server/Client System

Network programming and synchronization 

## Main/Bank App

Represents the bank server. There is a hashtable for the account management . The hashtable consists of buckets which can be accessed by one thread at a time (mutexes and condition variables) and each bucket is later filled with accounts (linked list) . 

The server waits for connections to a predifined port and when the request is accepted it is placed on a queue by the main thread. A thread from a thread pool of workers(the size of the thread pool is given from the command line) then becomes responsible for the client/server communication through the port until the client terminates the connection .Then it returns to the thread pool and awaits.


#### Arguments

 * flag -q thread pool size
 * flag -p port
 * flag -s queue size

## Bank Client

Connects to the bank server through a predifined port.Sends commands/instructions (file or stdin) to the server and waits for responses.

#### Arguments

 * flag -i file with instructions
 * flag -p port
 * flag -h host

#### Commands
(delay is optional)
* add_account [init_amount] [account_name] [delay]
   
> requests to create an account with name and initial amount of money
   * add_transfer [amount] [src_name] [dest_name] [delay]
    
> requests to create a transaction between the two accounts with given money amount
   * add_multi_transfer [amount] [src_name] [dst_name1] ... [dst_nameX] [delay]
      
> requests to create a transaction between the two accounts or more accounts with given money amount
   * print_balance [name]
    
> requests to print an account's balance
   * print_multi_balance [name1] ... [nameX]

> requests to print multiple accounts' balances
   * sleep [time]
    
> sleep-wait time milisecs
   * exit
      
> the client exits