#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//---------------------------------------Global Variables------------------------------------------//
pthread_mutex_t mtx;
pthread_cond_t inUse;
static struct {int amount; int available;} accountNum[2000];
static struct {int withdraw; int deposit; int value; } transaction[2000];
static int work=0;
static char buff[64];
static int totalcount = 0;
static int countAcc = 0;
static int transAmt = 0;
static int processed = 0;
int slices = 0;
int done=0;
//---------------------------------------Global Functions------------------------------------------//
static void listAccounts()
{
    for(int i = 0; i < countAcc; i++){
        printf("%d %d\n",i+1,accountNum[i].amount);
    }
}

static void getData(){
    while (fgets(buff, sizeof(buff),stdin) != NULL){
    char* tokmem = buff;
    //Until transactions start
    char* dumby = strtok_r(tokmem," ",&tokmem);
    char test = dumby[0];
    //printf("%c\n",test);
    if (test != 'T')
    {
    accountNum[countAcc].amount = atoi(strtok_r(NULL," ",&tokmem));
    countAcc++;
    }
    else if(test == 'T')
    {
    int newIn = totalcount-countAcc;
    
    transaction[newIn].withdraw = atoi(strtok_r(NULL," ",&tokmem));
    transaction[newIn].deposit = atoi(strtok_r(NULL," ",&tokmem));
    transaction[newIn].value = atoi(strtok_r(NULL," ",&tokmem));
    }
    totalcount++;
}
}

static void eft(int i, int id){

    int fromAcc = transaction[i].withdraw-1;
    int toAcc = transaction[i].deposit-1;
    int money = transaction[i].value;
    int s = 0;
    int no = 1;
    int leftAcc = accountNum[fromAcc].available;
    int rightAcc = accountNum[toAcc].available;

/*--------------Take Resources----------------*/
    //Acquire a mutex
        s = pthread_mutex_lock(&mtx);
            if(s!=0){
                perror("Mutex lock error");}
    //Check if resources are available
        while(leftAcc == no || rightAcc == no)
        {
           s = pthread_cond_wait(&inUse, &mtx);
            if(s!=0){
                perror("wait condition");}
        }
    //Set both resources to unavailable
        accountNum[fromAcc].available = no;
        accountNum[toAcc].available = no;

        s = pthread_mutex_unlock(&mtx);
            if(s!=0){
                perror("Mutex unlock error");}
        
    //Do the work
        accountNum[fromAcc].amount = accountNum[fromAcc].amount - money;
        accountNum[toAcc].amount = accountNum[toAcc].amount + money;
    /*Keep track of transactions processed succesffuly and trigger when done as to end main functions
    while loop waiting for thread computations to complete*/
        processed++;
        if (processed == transAmt){
            done=1;
        }

    /*----------------Put back----------------*/

        //Acquire the mutex
        s = pthread_mutex_lock(&mtx);
            if(s!=0){
                perror("Mutex lock error");}
        //set all acquried resources to available
        accountNum[fromAcc].available = 0;
        accountNum[toAcc].available = 0;
        //Broadcast resources available
        s = pthread_cond_broadcast(&inUse);
            if(s!=0){
                perror("wait condition");}
        //Release the mutex
        s = pthread_mutex_unlock(&mtx);
            if(s!=0){
                perror("Mutex lock error");}

}

//---------------------------------------Shared Thread Function------------------------------------------//
static void *
threadFunc(void *x)
{
int id = (int) x;    
//To distribute work load amongst threads using slices of total work
int start = id*slices;
int end = start + slices;
//Since the accounts is a prime number the slices can't be the same each time so to avoid
//Doing too many tansactions, I simply am hardcoding it to not give more than 17
if (end > transAmt+1){
    end = transAmt+1;
}

for(int i = start;i<end;i++){
    
    //Get next transaction line to perform
    eft(i,id);
}

}

//---------------------------------------Main Function------------------------------------------//
int
main(int argc, char *argv[])
{
    
// Read and initialize data
    getData();
    transAmt = totalcount-countAcc;

//Initialize pthreads and attributes   
    int numThreads = atoi(argv[1]); 
    pthread_t thr[numThreads];
    pthread_attr_t attr;
    int s;
    void *status;

    //Using numThreads arg to divide up slices of work
    if (numThreads != transAmt){
        slices = (transAmt/numThreads)+1;
    }
    else{
        slices = 1;
    }
    
//Set the Round Robin Attribute
    s = pthread_attr_init(&attr);       /* Assigns default values */
    if (s != 0)
        perror("pthread_attr_init");

    s = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (s != 0)
        perror("pthread_attr_setschedpolicy");

//Create the worker threads
    for(int i = 0;i<numThreads;i++){
        
        s = pthread_create(&thr[i], &attr, threadFunc, (void*) i);
        if (s != 0)
            perror("pthread_create");
    }
    
    while(done == 0){
        //Waiting for threads to complete all transactions before joining them
    }
    
     for(int i = 0;i<numThreads;i++){
         s = pthread_join(thr[i], &status);
         if (s != 0)
             perror("pthread_join failed");
 }

    s = pthread_attr_destroy(&attr);    /* No longer needed */
      if (s != 0)
          perror("pthread_attr_destroy");
        //clean up threads by joining

        //clean up mutex
    s = pthread_mutex_destroy(&mtx);
        if (s != 0)
          perror("pthread_attr_destroy");

    //Display final account balances
    listAccounts();
    return 0;
}

