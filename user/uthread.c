#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "uthread.h"

//task 1.1

static struct uthread pThreads[MAX_UTHREADS];
static struct uthread *currThread = 0;
int firstThread = 1;
int nextId = 1;

// // initialize the threads table.
// void
// threadinit(void){
//   struct uthread *t;
//   for(t = uthread; t < &uthread[MAX_UTHREADS]; t++) {
//       t->tstate = FREE;
//       t->ustack =(void *) malloc(STACK_SIZE);
//   }
// }


int uthread_create(void (*start_func)(), enum sched_priority priority){
    if(firstThread){
        struct uthread *t;
        for(t = uthread; t < &uthread[MAX_UTHREADS]; t++) {
            t->tstate = FREE;
            t->ustack =(void *) malloc(STACK_SIZE);
        }
        firstThread = 0;
    }
    int i;
    struct uthread *t;
    int found = 0;
    for(i = 0; i <MAX_UTHREADS && (found == 0) ; i++){
        if((&pThreads[i])->tid == 0){
            found = 1;
            break;
        }
    }
    if(found){
        return -1;
    }
    else{
        t = &pThreads[i];
        t->tid =nextId;
        nextId++;
        t->startFunc = start_func;
        t->priority = priority;
        t->state = RUNNABLE;
        void * myStack = (void*)malloc(STACK_SIZE);
        t->ustack = myStack + STACK_SIZE -1*(int) ;
        (t->context)->ra = start_func;
        (t->context)->sp = t->ustack;
    }

    return 0;
}