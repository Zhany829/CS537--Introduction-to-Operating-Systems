#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"


int
main(int argc, char **argv)
{
  if(argc != 3){
    printf(2, "usage: syscalls N g...\n");
    exit();
  }
  //if (*(argv[0]) < 1 || *(argv[1]) < 1) {
  //  printf(2, "usage: syscalls N g...\n");
   // exit();
  //}
  for(int i = 0; i < atoi((argv[2]))-1; i++){
    getpid();
   }
  for(int i = 0; i < atoi((argv[1]))-atoi((argv[2])); i++){
    kill(-1);
   }
   
  int pid = getpid();
  int num_syscalls = getnumsyscalls(pid);
  int num_good_syscalls = getnumsyscallsgood(pid);

  printf(1,"%d %d\n", num_syscalls, num_good_syscalls);
  exit();
}
