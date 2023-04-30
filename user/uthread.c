#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "uthread.h"

//task 1.1

static struct uthread pThreads[MAX_UTHREADS];
static struct uthread *currThread = 0;
int numOfCUrrThreads = 1;
int firstThread = 1;
int nextId = 1; 
struct uthread emptyThread;

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
    t->tid = nextId; 
    nextId++;
    t->index = i;
    t->startFunc = start_func;
    t->priority = priority;
    numOfCUrrThreads++;

    memset(&t->context, 0, sizeof(t->context));
    t->context.ra = (uint64)start_func;
    t->context.sp = (uint64)t->ustack + STACK_SIZE;

    t->state = RUNNABLE;
    return 0;
 }

 void uthread_yield(){
    struct uthread *old = currThread;
    int i = old->index;
    struct uthread *temp;
    struct uthread *t = 0;
    enum sched_priority priority = -1; 
    int j = 0;
    if(i < MAX_UTHREADS - 1)
       j = i + 1;
    temp = &pThreads[j];
    while (j != i){
        if(temp->state == RUNNABLE){
            if(priority < temp->priority){
                priority = temp->priority;
                t = temp;
            }
        }
        if(j == MAX_UTHREADS - 1){
            j = 0;
        }
        else{
            j++;
        } 
        temp = &pThreads[j];
    }
    if(t == 0)
        exit(0);
    old->state = RUNNABLE;
    t->state = RUNNING;
    currThread = t;
    uswtch(&old->context,&t->context);
 }


 void uthread_exit(){
    struct uthread *old = currThread;
    int i = old->index;
    struct uthread *temp;
    struct uthread *t = 0;
    enum sched_priority priority = -1; 
    int j = 0;
    if(i < MAX_UTHREADS - 1)
       j = i + 1;
    temp = &pThreads[j];
    while (j != i){
        if(temp->state == RUNNABLE){
            if(priority < temp->priority){
                priority = temp->priority;
                t = temp;
            }
        }
        if(j == MAX_UTHREADS - 1){
            j = 0;
        }
        else{
            j++;
        } 
        temp = &pThreads[j];
    }
    if(t == 0)
        exit(0);
    t->state = RUNNING;
    currThread = t;
    old->state = FREE;
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
            uswtch(&emptyThread.context,&t->context); 
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