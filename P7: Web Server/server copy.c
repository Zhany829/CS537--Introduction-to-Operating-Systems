#include "helper.h"
#include "request.h"
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
// #include "slot.h"
//
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

pthread_mutex_t mutex;
pthread_cond_t condition_condp;
pthread_cond_t condition_condc;
int idx = 0;               // buffer index
pthread_t *worker_pointer; // worker threads buffer pointer
int *buffer_pointer;       //

// Part B
// Shared connected file descriptor
int connfd_num = -1;
int buffer_size = -1;
int thread_size = -1;
char shard_name[100];
int port_num = -1;
int shared_listen = -1;
// slot_t *shm_slot_ptr;

void *consumer(void *arg)
{
  for (;;)
  {
    int fd = -1;
    pthread_mutex_lock(&mutex);
    while (idx == 0)
    {
      pthread_cond_wait(&condition_condc, &mutex);
    }
    // get fd here
    if (buffer_pointer[idx - 1] != -1)
    {
      fd = buffer_pointer[idx - 1];
      buffer_pointer[idx - 1] = -1;
      idx--;
    }
    pthread_cond_signal(&condition_condp); //not full
    pthread_mutex_unlock(&mutex);
    requestHandle(fd);

    Close(fd);
  }
  return NULL;
}

// void sigle_handler(int sig) {
//   if (munmap(shm_ptr, PAGESIZE) == -1)
//   {
//     fprintf(stderr, "Error occur when call munmap.\n");
//     exit(1);
//   }
//   if (shm_unlink(shm_fd) == -1)
//   {
//     fprintf(stderr, "Error occur when call shm_unlink.\n");
//     exit(1);
//   }
// }

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    fprintf(stderr, "Usage: %s <port> <threads> <buffers> <shm_name>\n", argv[0]);
    exit(1);
  }
  pthread_cond_init(&condition_condp, NULL);
  pthread_cond_init(&condition_condc, NULL);
  pthread_mutex_init(&mutex, NULL);
  int listenfd, connfd, clientlen;
  struct sockaddr_in clientaddr;
  char shm_name[100];
  int port_num, worker_threads_num, buffer_num;
  port_num = atoi(argv[1]);
  worker_threads_num = atoi(argv[2]);
  buffer_num = atoi(argv[3]);
  strcpy(shm_name, argv[4]);
  buffer_size = buffer_num;
  thread_size = worker_threads_num;
  strcpy(shard_name, shm_name);
  if (port_num < 2000 || port_num > 65535)
  {
    fprintf(stderr, "The port number should between 2000 and 65535");
    exit(1);
  }
  if (worker_threads_num <= 0 || buffer_num <= 0)
  {
    fprintf(stderr, "Number of worker threads or request connection must be positive.\n");
    exit(1);
  }
  // CS537 (Part B): Create & initialize the shared memory region...
  // 1. Create
  // int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0660);
  // if (shm_fd < 0) {
  //   perror("shm_open_1 fail");
  //   return 1;
  // }
  // // 2. Initialize
  // int pagesize = getpagesize();
  // if (ftruncate(shm_fd, pagesize) != 0)
  // {
  //   fprintf(stderr, "Error occur when initialize the shared memory region.\n");
  //   exit(1);
  // }
  // 3. Map
  // shm_fd = shm_open(shm_name, O_RDWR, 0660);
  // void *shm_ptr = mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  // typedef struct
  // {
  //   char str[32];
  //   int num;
  // } slot_t;

  // slot_t *shm_slot_ptr = (slot_t *)shm_ptr;
  // if (shm_slot_ptr == NULL) {
  //   fprintf(stderr, "Setup fail.\n");
  //   exit(1);
  // }
  // // 4. Delete
  // signal(SIGINT, signal_handler); // create a function signal handler?

  //
  // CS537 (Part A): Create some threads...
  //
  int work_buffer[buffer_num]; // connfd buffer
  // consumer
  pthread_t workers[worker_threads_num];
  for (int i = 0; i < worker_threads_num; i++)
  {
    // pthread_t i;
    pthread_create(&workers[i], NULL, &consumer, NULL);
  }
  // Link the worker thread pointer to the worker thread array
  worker_pointer = workers;
  // Initialize work buffer
  for (int i = 0; i < buffer_num; i++)
  {
    work_buffer[i] = -1;
  }
  // Link the work buffer pointer to the work buffer array
  buffer_pointer = work_buffer;

  listenfd = Open_listenfd(port_num);
  shared_listen = listenfd;
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads
    // do the work. Also let the worker thread close the connection.
    pthread_mutex_lock(&mutex);
    while (idx == buffer_num)
    { // Wait on full buffer
      pthread_cond_wait(&condition_condp, &mutex);
    }
    if ((idx < buffer_num) && (work_buffer[idx] == -1))
    {
      work_buffer[idx] = connfd;
      idx++;
    }
    pthread_cond_signal(&condition_condc); //not empty
    pthread_mutex_unlock(&mutex);
  }
}
