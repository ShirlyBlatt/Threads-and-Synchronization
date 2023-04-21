#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "uthread.h"

//task 1.1

static struct uthread pThreads[MAX_UTHREADS];
static struct uthread *currThread = 0;
int numOfCUrrThreads = 1;
int firstThread = 1;
int nextId = 1; //maybe we dont need TODO

void freeThread(struct uthread* t){
    int i;
    for (i=0; i<MAX_UTHREADS; i++){
		if(t->tid == (&pThreads[i])->tid){
            free(t->ustack);
            (&pThreads[i])->state = FREE;
            numOfCUrrThreads--;
            (&pThreads[i])->tid = 0;
            (&pThreads[i])->startFunc = 0;
            (&pThreads[i])->priority = 0;
            return;
        }
    }
}

 int uthread_create(void (*start_func)(), enum sched_priority priority){
    int i;
    struct uthread *t;
    int found = 0;
    if(numOfCUrrThreads == MAX_UTHREADS){
        return -1;
    }
    for(i = 0; i <MAX_UTHREADS && (found == 0) ; i++){
        if((&pThreads[i])->state == FREE){
            found = 1;
            break;
        }
    }
    if(!found){
        return -1;
    }
    t = &pThreads[i];
    t->tid =nextId; //maybe we dont need TODO
    nextId++;
    t->startFunc = start_func;
    t->priority = priority;
    void * myStack = (void*)malloc(STACK_SIZE); //TODO
    t->ustack[0] = myStack; 
    t->context.ra =(uint64)uthread_exit;
    t->context.sp = (uint64)&(t->ustack[STACK_SIZE]);
    t->context.sp -= sizeof(uint64);
    *((uint64*)(t->context.sp)) = (uint64)start_func;
    t->state = RUNNABLE;
    numOfCUrrThreads++;
    return 0;
 }

 void uthread_yield(){
    int i;
    struct uthread *old = uthread_self();
    struct uthread *t = old;
    int found = 0;
    for(i = 0; i <MAX_UTHREADS && (found == 0) ; i++){
        if(t->tid == i){
           found = 1;
           break;
        }
    }
    int j=i+1;
    while (j != i){
        if(j == MAX_UTHREADS){
            j = 0;
        }
        if(t->priority < (&pThreads[j])->priority){
            if((&pThreads[j])->state == RUNNABLE){
                t = &pThreads[j];
            }
        }
        j++;
    }
    t->state = RUNNING;
    currThread = t;
    old->state = RUNNABLE;
    uswtch(&old->context,&t->context);
 }


 void uthread_exit(){
    int i;
    struct uthread *old = uthread_self();
    struct uthread *t = old;
    int found = 0;
    for(i = 0; i <MAX_UTHREADS && (found == 0) ; i++){
        if(t->tid == i){
           found = 1;
           break;
        }
    }
    int j=i+1;
    while (j != i){
        if(j == MAX_UTHREADS){
            j = 0;
        }
        if(t->priority < (&pThreads[j])->priority){
            if((&pThreads[j])->state == RUNNABLE){
                t = &pThreads[j];
            }
        }
        j++;
    }
    t->state = RUNNING;
    currThread = t;
    freeThread(old);
    if(numOfCUrrThreads == 1){
        exit(0);
    }
    uswtch(&old->context,&t->context);
 }

 int uthread_start_all(){
    if(firstThread){
        firstThread = 0;
        int i;
        struct uthread *t = 0;
        int first = 1;
        for (i = 0; i<MAX_UTHREADS; i++){
            if((&pThreads[i])->state == RUNNABLE){
                if(first){
                    t = &pThreads[i];
                    first = 0;
                }
                else{
                    if((&pThreads[i])->priority > t->priority){
                        t = &pThreads[i];
                    }
                }
            }
        }
        if(t != 0){
            t->state = RUNNING;
            currThread = t;
            uswtch(0,&t->context); //TODO check if currect to put 0
        }
    }
    return -1;
 }

enum sched_priority uthread_set_priority(enum sched_priority priority){
    struct uthread *t = uthread_self();
    enum sched_priority oldPriority = t->priority; 
    t->priority = priority;
    return oldPriority;
}

enum sched_priority uthread_get_priority(){
    struct uthread *t = uthread_self();
    return t->priority;
}


struct uthread* uthread_self(){
    return currThread;
}