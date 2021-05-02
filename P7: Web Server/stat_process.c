#include "helper.h"
#include "request.h"
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include "shared-memory-slot.h"
void signal_handler();
slot_t *shm_ptr;
char shm_name[50];
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s [shm_name] [sleeptime_ms] [num_threads]\n", argv[0]);
        exit(1);
    }
    signal(SIGINT, signal_handler);
    int sleeptime_ms;
    int worker_threads_num;
    strcpy(shm_name, argv[1]);
    sleeptime_ms = atoi(argv[2]);
    worker_threads_num = atoi(argv[3]);
    if (worker_threads_num <= 0 || sleeptime_ms <= 0)
    {
        fprintf(stderr, "Number of worker threads or sleep time must be positive.\n");
        exit(1);
    }

    // struct timespec ts;
    // ts.tv_sec = 0;
    // ts.tv_nsec = sleeptime_ms * 1000;

    int pagesize = getpagesize();
    // Create the shared memory region.
    int shmfd = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
    if (shmfd < 0)
    {
        fprintf(stderr, "shm_open failed.\n");
        exit(1);
    }
    shm_ptr = mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm_ptr == NULL)
    {
        perror("mmap failed.\n");
        exit(1);
    }
    int count = 1;
    // Begin continuously creating threads and consumes requests.
    while (1)
    {
        sleep(sleeptime_ms / 1000);
        for (int i = 0; i < worker_threads_num; i++) {
            fprintf(stdout, "%d\n%lu : %d %d %d\n", count, shm_ptr[i].threadNum,
                    shm_ptr[i].requestCompleted, shm_ptr[i].staticNum, shm_ptr[i].dynamicNum);
        }
        count++;
    }
    return 0;
}

// int numIteration = 0;
// for(;;) {
//   numIteration++;
//   nanosleep(&ts, NULL);

//   // Memory map.
//   slot_t *shm_ptr = mmap(NULL, pagesize, PROT_READ, MAP_SHARED, shmfd, 0);
//   if (shm_ptr == MAP_FAILED) {
//       fprintf(stderr, "mmap failed.\n");
//       exit(1);
//   }

//   // Read the memory and print
//   fprintf(stdout, "\n%d", numIteration);
//   for(int i=0; i<worker_threads_num; i++) {
//     fprintf(stdout, "%d : %d %d %d\n", shm_ptr[i].threadNum, shm_ptr[i].requestCompleted, shm_ptr[i].staticNum, shm_ptr[i].dynamicNum);
//   }

//   // Unmap.
//   int ret = munmap(shm_ptr, pagesize);
//   if (ret != 0) {
//       fprintf(stderr, "munmap failed.\n");
//       exit(1);
//   }
// }
// return 0;
//}
// Handles the SIGINT interrupt
void signal_handler() {
    if (munmap(shm_ptr, getpagesize()) != 0)
    {
        perror("munmap failed.\n");
        exit(1);
    }

    if (shm_unlink(shm_name) != 0)
    {
        perror("shm_unlink failed.\n");
        exit(1);
    }
    exit(0);
}