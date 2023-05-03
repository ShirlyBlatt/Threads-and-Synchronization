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
    // return 0;

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

// void kthread_exit(int status)
// {
//   struct proc *p = myproc();
//   struct kthread *kt = mykthread();
//   int i;

//   // Set the current kernel thread's status to K_ZOMBIE and set its exit status
//   acquire(&kt->lock);
//   kt->xstate = status;
//   kt->state = K_ZOMBIE;
//   release(&kt->lock);

//   // Check if any kernel thread in this process is still active
//   acquire(&p->lock);
//   for (i = 0; i < NKT; i++){
//     struct kthread *kt = &p->kthread[i];
//     acquire(&kt->lock);
//     if (kt->state != K_ZOMBIE && kt->state != K_UNUSED){
//       // Found a kernel thread that is not a zombie
//       release(&kt->lock);
//       break;
//     }
//     release(&kt->lock);
//   }

//   if (i == NKT){ // All kernel threads in this process are zombies
//     release(&p->lock);
//     exit(status); // Terminate the process and set its exit status to the given status
//   }
//   else{ // At least one kernel thread is not a zombie
//     kt = mykthread();
//     acquire(&kt->lock);
//     kt->state = K_ZOMBIE;
//     kt->xstate = status;
//     release(&p->lock);

//     sched(); // kt->lock should be acquired when calling sched()
//     panic("should not print this\n");
//   }
// }

// void
// exit(int status)
// {
//   struct proc *p;
//   struct kthread *kt;

//   // Make sure only one thread execute exit()
//   static int first = 1;
//   if (first) {
//     p = myproc();

//     // Go over the threads and turn on their kill flag, besides the current thread.
//     // The Zombie status will be set in kthread_exit() which is called by usertrap() in trap.c.
//     for(kt = p->kthread; kt < &p->kthread[NKT]; kt++){
//       if((kt->tid != mykthread()->tid) & ((kt->state != K_UNUSED) && (kt->state != K_ZOMBIE))) { 
//           acquire(&kt->lock);
//           kt->killed=1;
//           release(&kt->lock);
//           kthread_join(kt->tid, 0);
//         }
//     }

//     if(p == initproc)
//       panic("init exiting");

//     // Close all open files.
//     for(int fd = 0; fd < NOFILE; fd++){
//       if(p->ofile[fd]){
//         struct file *f = p->ofile[fd];
//         fileclose(f);
//         p->ofile[fd] = 0;
//       }
//     }

//     begin_op();
//     iput(p->cwd);
//     end_op();
//     p->cwd = 0;

//     acquire(&wait_lock);

//     // Give any children to init.
//     reparent(p);

//     // Parent might be sleeping in wait().
//     wakeup(p->parent);
    
//     // Update the state and xstate of the process
//     acquire(&p->lock);
//     p->xstate = status;
//     p->state = P_ZOMBIE;

//     release(&p->lock);

//     kt = mykthread();
//     acquire(&kt->lock); // We will not release kt->lock in exit(), because when calling sched() kt->lock should be acquired
//     kt-> xstate = status;
//     kt->state = K_ZOMBIE;

//     release(&wait_lock);
//   }

//   // If exit() was already executed by another thread
//   else {
//     acquire(&mykthread()->lock);
//     kthread_exit(0);
//   }
// // Jump into the scheduler, never to return.
//   sched();
//   panic("zombie exit");
// }