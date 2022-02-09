#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


int sizeOfBuffer = 4;
int buffer[4];

int counter = 0;
sem_t c, s, emptySlots, fullSlots; 

void * counterFunction(void * args)
{
   int counterThreadIndex   =  *((int *)args);    //takes the index of the counter thread as an argument
   while(1)
   {
      printf("Counter Thread %d (%ld): received a message\n", counterThreadIndex, pthread_self());   //pthread_self() gets the thread ID
      printf("Counter Thread %d (%ld): waiting to write\n", counterThreadIndex, pthread_self());
      sem_wait(&c);        //counter thread waits (decrements) on the counter semaphore so it can enter the critical section
      counter++;           
      printf("Counter Thread %d (%ld): now adding to counter, counter value = %d\n", counterThreadIndex, pthread_self(), counter);
      sem_post(&c);        //after leaving the critical section, it increments the counter semaphore (semSignal)
     
      int randomNo = random() % 10;
      sleep(randomNo);
   
   }
}

void * monitorFunction(void * args)
{
   int received, Monitor_BufferIndex = 0;
   
   while(1)
   {
      printf("Monitor Thread: Waiting to read counter\n");
      sem_wait(&c);        //monitor thread waits (decrements) on the counter semaphore so it can enter the critical section
      received = counter;  //saves the current value of the counter and resets it to 0
      counter = 0;
      printf("Monitor Thread: reading a count value of %d\n", received);
      sem_post(&c);       //after leaving the critical section, it increments the counter semaphore (semSignal)
      
      
      if(Monitor_BufferIndex == sizeOfBuffer)       //resets the buffer index if it reaches the end of the buffer
      Monitor_BufferIndex = 0;
      
      int value;
      sem_getvalue(&emptySlots, &value);
      if(value <= 0)                                //if there are no empty slots in the buffer, then it's full 
      printf("Monitor Thread: Buffer is Full!\n");
      
      sem_wait(&emptySlots);                        //decrements emptySlots before adding to the buffer
      sem_wait(&s);                                 //monitor thread waits (decrements) on the semaphore so it can enter the critical section
      buffer[Monitor_BufferIndex] = received;       //puts the received counter value in the buffer
      printf("Monitor Thread: Writing to the buffer at position %d\n", Monitor_BufferIndex);
      sem_post(&s);                                 //after leaving the critical section, it increments the semaphore (semSignal)
      sem_post(&fullSlots);                         //increments fullSlots after adding to the buffer
      
      Monitor_BufferIndex++;
      
      int randomNo = random() % 10;
      sleep(randomNo);
   
   }
}

void * collectorFunction(void * args)
{
   int collector_BufferIndex = 0;
   while(1)
   {  
      if(collector_BufferIndex == sizeOfBuffer)       //resets the buffer index if it reaches the end of the buffer
      collector_BufferIndex = 0;
      
      int value;
      sem_getvalue(&fullSlots, &value);
      if(value <= 0)                                 //if there are no full slots in the buffer, then it's empty 
      printf("Collector Thread: Nothing is in the Buffer!\n");
      
      sem_wait(&fullSlots);                         //decrements fullSlots before removing from the buffer
      sem_wait(&s);                                 //collector thread waits (decrements) on the semaphore so it can enter the critical section
      buffer[collector_BufferIndex] = 0;            //resets the value of the current slot in the buffer
      printf("Collector Thread: Reading from the buffer at position %d\n", collector_BufferIndex);
      sem_post(&s);                                //after leaving the critical section, it increments the semaphore (semSignal)
      sem_post(&emptySlots);                       //increments emptySlots after removing from the buffer
      
      collector_BufferIndex++;
      
      int randomNo = random() % 10;
      sleep(randomNo);
   
   }
}


int main(){

int n;
sem_init(&c, 0, 1);                          //initialises counter semaphore (c) with 1 
sem_init(&s, 0, 1);                          //initialises semaphore (s) with 1 
sem_init(&emptySlots, 0, sizeOfBuffer);      //initialises emptySlots semaphore with the size of the buffer 
sem_init(&fullSlots, 0, 0);                  //initialises fullSlots semaphore with 0 

printf("Enter No. of Counter Threads:");
scanf("%d", &n);

srandom(time(0));      //uses current time as a seed for the random number generator
pthread_t counter[n], monitor, collector;
int *threadIndex = malloc(n * sizeof(int));   

for(int i = 0; i < n; i++)
{ 
   threadIndex[i] = i;
   pthread_create(&counter[i], NULL, &counterFunction, (void *)&threadIndex[i]);
}
pthread_create(&monitor, NULL, &monitorFunction, NULL);
pthread_create(&collector, NULL, &collectorFunction, NULL);

pthread_join(monitor, NULL);     //only one join is needed since all threads run in an infinite loop

return 0;
}
