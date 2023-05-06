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


void* func(){
    kthread_exit(0);
    return 0;
}

int main(int argc, char** argv){
    printf("This test will try to call for kthread_exit from 3 threads thread\nExpected Behvaiour: sucessfull exit and return to the shell\n");
    uint64 memory = (uint64)malloc(4000);
    uint64 memory2 = (uint64)malloc(4000);
    int tid = kthread_create(func,memory,4000);
    fprintf(2,"[main] created %d\n",tid);
    tid = kthread_create(func,memory2,4000);
    fprintf(2,"[main] created %d\n",tid);
    kthread_exit(0);
    return 1;
}