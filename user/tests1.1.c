#include "../kernel/param.h"
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "../kernel/fs.h"
#include "../kernel/fcntl.h"
#include "../kernel/syscall.h"
#include "../kernel/memlayout.h"
#include "../kernel/riscv.h"
#include "uthread.h"



// #include "../kernel/types.h"
// #include "user.h"
// #include "../kernel/stat.h"
// #include "uthread.h"




void thread_start_func(void){
  for(int i=0; i<10; i++){
    sleep(10); // simulate work
  }
  kthread_exit(0);
  printf("kthread_exit failed\n");
  exit(1);
}


// void hello1(){
//     printf("Hello World from hello1");
//     uthread_exit();
// }

// void hello2(){
//     printf("Hello World from hello2");
//     uthread_exit();
// }



int main(int argc, char *argv[]){
    // printf("Hello World from main\n");    
    // uthread_create(&hello1, LOW);
    // uthread_create(&hello2, HIGH);
    // uthread_start_all();
    // printf("after all\n");
//     return 0;
// }

//tests kllt
  uint64 stack_a = (uint64)malloc(MAX_STACK_SIZE);
  uint64 stack_b = (uint64)malloc(MAX_STACK_SIZE);

  int kt_a = kthread_create((void *(*)())thread_start_func, stack_a, MAX_STACK_SIZE);
  if(kt_a <= 0){
    printf("kthread_create failed\n");
    exit(1);
  }
  printf("after first create");
  int kt_b = kthread_create((void *(*)())thread_start_func, stack_b, MAX_STACK_SIZE);
  if(kt_a <= 0){
    printf("kthread_create failed\n");
    exit(1);
  }
  printf("after second create");
  int joined = kthread_join(kt_a, 0);
  if(joined != 0){
    printf("kthread_join failed\n");
    exit(1);
  }
    printf("after first join");
  joined = kthread_join(kt_b, 0);
  if(joined != 0){
    printf("kthread_join failed\n");
    exit(1);
  }
   printf("after second join");

  free((void *)stack_a);
  free((void *)stack_b);
  printf("finished");
return 0;
}

