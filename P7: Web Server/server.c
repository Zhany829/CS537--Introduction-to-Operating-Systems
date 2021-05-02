#include "helper.h"
#include "request.h"
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "shared-memory-slot.h"
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
int idx = 0; // buffer index
pthread_t *worker_pointer; // worker threads buffer pointer
int *buffer_pointer; // 

// Part B
int slot_idx = 0;
// Shared connected file descriptor
int connfd_num = -1;
int buffer_size = -1;
int thread_size = -1;
char shard_name[100];
int port = -1;
int shared_listen = -1;
slot_t *shm_ptr;

void *consumer(void *arg) {
  for (;;) {
    int fd = -1;
    pthread_mutex_lock(&mutex);
    while (idx == 0) {
      pthread_cond_wait(&condition_condc, &mutex);
    }
    // get fd here
    if (buffer_pointer[idx - 1] != -1) {
      fd = buffer_pointer[idx - 1];
      buffer_pointer[idx - 1] = -1;
      idx--;
    }
    pthread_cond_signal(&condition_condp); //not full
    pthread_mutex_unlock(&mutex);
    // Used to count the shm_ptr occupied position
    int index1 = -1;
    for (int i = 0; i < slot_idx; i++) {
      if (shm_ptr[i].threadNum == pthread_self()) {
        index1 = i;
        break;
      }
    }
    if (index1 == -1) {
      index1 = slot_idx;
      slot_idx++;
      shm_ptr[index1].threadNum = pthread_self();
    }
    shm_ptr[index1].requestCompleted++;
    int ret = requestHandle(fd);
    if (ret == -1) {
      return NULL;
    } else if (ret == 1) {
      shm_ptr[index1].dynamicNum += 1;
    } else if (ret == 0) {
      shm_ptr[index1].staticNum += 1;
    }
    Close(fd);
  }
  return NULL;
}

void signal_handler(int sig) {
  if (munmap(NULL, getpagesize()) == -1) {
    fprintf(stderr, "Error occur when call munmap.\n");
    exit(1);
  }
  if (shm_unlink(shard_name) == -1) {
    fprintf(stderr, "Error occur when call shm_unlink.\n");
    exit(1);
  }
  close(shared_listen);
  // Destroy mutex and CV
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condition_condp);
  pthread_cond_destroy(&condition_condc);
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
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
  // parse arguments
  port_num = atoi(argv[1]);
  worker_threads_num = atoi(argv[2]);
  buffer_num = atoi(argv[3]);
  strcpy(shm_name, argv[4]);
  // share value
  buffer_size = buffer_num;
  thread_size = worker_threads_num;
  port = port_num;
  strcpy(shard_name, shm_name);
  if (port_num < 2000 || port_num > 65535) {
    fprintf(stderr, "The port number should between 2000 and 65535");
    exit(1);
  }
  if (worker_threads_num <= 0 || buffer_num <= 0) {
    fprintf(stderr, "Number of worker threads or request connection must be positive.\n");
    exit(1);
  }
  // CS537 (Part B): Create & initialize the shared memory region...
  // 1. Create
  int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0660);
  if (shm_fd < 0) {
    perror("shm_open_1 fail");
    return 1;
  }
  // 2. Initialize
  int pagesize = getpagesize();
  if (ftruncate(shm_fd, pagesize) != 0) {
    fprintf(stderr, "Error occur when initialize the shared memory region.\n");
    exit(1);
  }
  // 3. Map
  // shm_fd = shm_open(shm_name, O_RDWR, 0660);
  shm_ptr = mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == NULL) {
    fprintf(stderr, "mmap failure.\n");
    exit(1);
  }
  // initialize shm_ptr
  int slot_size = getpagesize() / sizeof(slot_t);
  for (int i = 0; i < slot_size; i++) {
    shm_ptr[i].threadNum = -1;
    shm_ptr[i].requestCompleted = 0;
    shm_ptr[i].staticNum = 0;
    shm_ptr[i].dynamicNum = 0;
  }
  // 4. Delete
  signal(SIGINT, signal_handler); 
  //
  // CS537 (Part A): Create some threads...
  //
  int work_buffer[buffer_num]; // connfd buffer
  // consumer
  pthread_t workers[worker_threads_num];
  for (int i = 0; i < worker_threads_num; i++) {
    // pthread_t i;
    pthread_create(&workers[i], NULL, &consumer, NULL);
  }
  // Link the worker thread pointer to the worker thread array
  worker_pointer = workers;
  // Initialize work buffer
  for (int i = 0; i < buffer_num; i++) {
    work_buffer[i] = -1;
  }
  buffer_pointer = work_buffer;

  listenfd = Open_listenfd(port_num);
  shared_listen = listenfd;
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads
    // do the work. Also let the worker thread close the connection.
    pthread_mutex_lock(&mutex);
    while (idx == buffer_num) { // Wait on full buffer
      pthread_cond_wait(&condition_condp, &mutex);
    }
    if ((idx < buffer_num) && (work_buffer[idx] == -1)) {
      work_buffer[idx] = connfd;
      idx++;
    }
    pthread_cond_signal(&condition_condc); //not empty
    pthread_mutex_unlock(&mutex);
  }
}

