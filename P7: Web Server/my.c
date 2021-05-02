#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER;

void *producer();
void *consumer();
// int  count = 0;
int numempty = 0;
int numfull = 0;
// #define COUNT_DONE  10
// #define COUNT_HALT1  3
// #define COUNT_HALT2  6

main()
{
    char *shm_name;
    int port_num;
    int worker_threads_num;
    int buffer_num;
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
        exit(1);
    }
    port_num = atoi(argv[1]);
    worker_threads_num = argv[2];
    buffer_num = argv[3];
    shm_name = atoi(argv[4]);
    if (worker_threads_num <= 0 || buffer_num <= 0) {
        fprintf(stderr, "Number of worker threads or request connection must be positive.\n");
        exit(1);
    }
    int *work_buffer[buffer_num];  // connfd buffer

    // producer
    pthread_t producer;
    pthread_create( &producer, NULL, &producer(work_buffer), NULL);
    // consumer
    for (int i = 0; i < worker_threads_num; i++) {
        pthread_t i;
        pthread_create( &i, NULL, &consumer(work_buffer), NULL);
        pthread_join( producer, NULL);
        pthread_join( i, NULL);
    }
   exit(0);
}

void *producer(int *buffer)
{
   for(;;)
   {
      pthread_mutex_lock( &condition_mutex );
      while(numempty == 0)
      {
         pthread_cond_wait( &condition_cond, &condition_mutex );
      }
      // get fd here
      int index = 0;
      while (buffer[index++] != NULL);
      pthread_mutex_unlock( &condition_mutex );
      // fill buffer here + update index
      buffer[index] = fd; // how to get fd to put?

      pthread_mutex_lock( &count_mutex );
      numfull++;
      pthread_cond_signal( &condition_cond );//not empty
      pthread_mutex_unlock( &count_mutex );

      return;
    }
}

void *consumer(int *buffer)
{
    for(;;)
    {
       pthread_mutex_lock( &condition_mutex );
       while(numfull == 0)
      {
         pthread_cond_wait( &condition_cond, &condition_mutex );
      }
       // get fd here
      int index = 0;
      while (buffer[index++] == NULL);
      int fd = buffer[index];
      pthread_mutex_unlock( &condition_mutex );
      pthread_mutex_unlock( &condition_mutex );

      pthread_mutex_lock( &count_mutex );
      numempty++;
      pthread_cond_signal( &condition_cond );//not full
      pthread_mutex_unlock( &count_mutex );

       return;
    }

}