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



// // #include "../kernel/types.h"
// // #include "user.h"
// // #include "../kernel/stat.h"
// // #include "uthread.h"




// void thread_start_func(void){
//   for(int i=0; i<10; i++){
//     sleep(10); // simulate work
//   }
//   kthread_exit(0);
//   printf("kthread_exit failed\n");
//   exit(1);
// }


// // void hello1(){
// //     printf("Hello World from hello1");
// //     uthread_exit();
// // }

// // void hello2(){
// //     printf("Hello World from hello2");
// //     uthread_exit();
// // }



// int main(int argc, char *argv[]){
//     // printf("Hello World from main\n");    
//     // uthread_create(&hello1, LOW);
//     // uthread_create(&hello2, HIGH);
//     // uthread_start_all();
//     // printf("after all\n");
// //     return 0;
// // }

// //tests kllt
//   uint64 stack_a = (uint64)malloc(MAX_STACK_SIZE);
//   uint64 stack_b = (uint64)malloc(MAX_STACK_SIZE);

//   int kt_a = kthread_create((void *(*)())thread_start_func, stack_a, MAX_STACK_SIZE);
//   if(kt_a <= 0){
//     printf("kthread_create failed\n");
//     exit(1);
//   }
//   printf("after first create");
//   int kt_b = kthread_create((void *(*)())thread_start_func, stack_b, MAX_STACK_SIZE);
//   if(kt_a <= 0){
//     printf("kthread_create failed\n");
//     exit(1);
//   }
//   printf("after second create");
//   int joined = kthread_join(kt_a, 0);
//   if(joined != 0){
//     printf("kthread_join failed\n");
//     exit(1);
//   }
//     printf("after first join");
//   joined = kthread_join(kt_b, 0);
//   if(joined != 0){
//     printf("kthread_join failed\n");
//     exit(1);
//   }
//    printf("after second join");

//   free((void *)stack_a);
//   free((void *)stack_b);
//   printf("finished");
// return 0;
// }


// // test kthread create:
// void* func(){
//     kthread_exit(0);
//     return 0;
// }

// int main(int argc, char** argv){
//     printf("This test will try to call for kthread_create\nExpected Behvaiour: the new thread's id should be 2.\nfor further testing, one should run this test while debugging allocproc after running ls.\nThis is in order to check that the main thread is the first thread in the kthread array\n");
//     uint64 memory = (uint64)malloc(4000);
//     int tid = kthread_create(func,memory,4000);
//     fprintf(2,"[main] created %d\n",tid);
//     if(tid != 2){
//         fprintf(2,"[ERROR] got tid=%d,should have been 1\n",tid);
//         return 0x66;
//     }
//     kthread_exit(0);
//     return 1;
// }


// // test kthread exec:
// void* fib(){
//     int t1=0;
//     int t2=1;
//     int nextTerm=t1+t2;
//     for(int i = 0;i<100;i++){
//         t1=t2;
//         t2=nextTerm;
//         nextTerm=t1+t2;
//     }
//     printf("fib 100 = %d\n",nextTerm);
//     kthread_exit(0);
//     return 0;
// }
// void* exectest(){
//     char* command = "echo";
//     char *argv[] = {"echo","Success!\n",0};
//     printf("in exectest\n");
//     exec(command,argv);
//     fprintf(2,"[ERROR] kthread %d has returned\n",kthread_id());
//     return 0;
// }

// int main(int argc, char** argv){
//     printf("This test will try to call for exec from 3 different threads\nExpected behvaiour: printing Success via a call to exec to run the command echo Success\n");
//     uint64 memory = (uint64)malloc(4000);
//     uint64 memory2 = (uint64)malloc(4000);
//     kthread_create(exectest,memory,4000);
//     kthread_create(exectest,memory2,4000);
//     printf("terminate creating\n");
//     exectest();
//     fprintf(2,"[ERROR] kthread %d has returned\n",kthread_id());

//     return 1;
// }

// //test kthread exit
// void* func(){
//     kthread_exit(0);
//     return 0;
// }

// int main(int argc, char** argv){
//     printf("This test will try to call for kthread_exit from 3 threads thread\nExpected Behvaiour: sucessfull exit and return to the shell\n");
//     uint64 memory = (uint64)malloc(4000);
//     uint64 memory2 = (uint64)malloc(4000);
//     int tid = kthread_create(func,memory,4000);
//     fprintf(2,"[main] created %d\n",tid);
//     tid = kthread_create(func,memory2,4000);
//     fprintf(2,"[main] created %d\n",tid);
//     kthread_exit(0);
//     return 1;
// }

// //test fork
// void chld(){
//     printf("Just a perfect day, drink Sangria in the park\ //
//     \nAnd then later, when it gets dark, we go home\n\  //
// Just a perfect day, feed animals in the zoo\n\   //
// Then later, a movie too and then home\n\n\  //
// Oh, it's such a perfect day\n\  //
// I'm glad I spent it with you\n\  //
// Oh, such a perfect day\n\  //
// You just keep me hanging on\n\  //
// You just keep me hanging on\n\n\  //
// Just a perfect day, problems all left alone\n\  //
// Weekenders on our own, it's such fun\n\  //
// Just a perfect day, you made me forget myself\n\  //
// I thought I was someone else, someone good\n\n\  //
// Oh, it's such a perfect day\n\  //
// I'm glad I spent it with you\n\  //
// Oh, such a perfect day\n\  //
// You just keep me hanging on\n\  //
// You just keep me hanging on\n"); //
// exit(1);
// }
// void* thread(){
//     int status;
//     int pid;
//     if((pid = fork()) ==0){
//         fprintf(2,"child's PID: %d\n",getpid());
//         chld();
//     }else{
//         if(pid == getpid()){
//             fprintf(2,"[ERROR] the new process is the same as the other process\n");   
//             exit(0x55); 
//         }
//         if(wait(&status) != -1 && status != 1){
//                 fprintf(2,"[ERROR] child exited abnormally\n");   
//                 kthread_exit(-1);         
//         }
//         kthread_exit(0);
//     }
//     return 0;
// }
// int main(int argc,char** argv){
//     printf("This test will try to fork this process from a thread.\n\ //
//     Expected Behaviuor:\n\ //
//     the child prints the lyrics of the song Perfect Day by Lou Reed and exit\n\n");
//     fprintf(2,"father PID: %d\n",getpid());
//     uint64 stack = (uint64)malloc(4000);
//     //int thread_status;
//     int tid;
//     if((tid = kthread_create(thread,stack,4000)) == -1){
//         fprintf(2,"[ERROR] couldn't start a thread\n");
//         return 1;
//     }
//     return 0;
// }

// // // test thread kill join
// void* fib1000000000(){
//     int t1=0;
//     int t2=1;
//     int nextTerm=t1+t2;
//     for(int i = 0;i<1000000000;i++){
//         t1=t2;
//         t2=nextTerm;
//         nextTerm=t1+t2;
//     }
//     //printf("fib 1000000000 = %d\n",nextTerm);
//     kthread_exit(1);
//     return 0;
// }
// void* fib100(){
//     int t1=0;
//     int t2=1;
//     int nextTerm=t1+t2;
//     for(int i = 0;i<100;i++){
//         t1=t2;
//         t2=nextTerm;
//         nextTerm=t1+t2;
//     }
//     printf("fib 100 = %d\n",nextTerm);
//     kthread_exit(1);
//     return 0;
// }
// void* forktest(){
//     char* command = "echo";
//     char *argv[] = {"echo","testing fork from a multithreaded process",0};
//     exec(command,argv);
//     fprintf(2,"[ERROR]: shouldn't returned\n");
//     return 0;
// }

// int main(int argc, char** argv){
//     printf("This test will try to kill and join a thread\nExpected Behaviour:\nMain thread creates a thread which try to calculate fib 1000000000, then it will kill it as well as join it.\n the expected status should be 1.\nPossible Errors: Main thread doesn't return from join.\n");
//     uint64 memory = (uint64)malloc(4000);
//     uint64 memory2 = (uint64)malloc(4000);
//     int returned_status;
//     int tid = kthread_create(fib1000000000,memory,4000);
//     kthread_kill(tid);
//     if(kthread_join(tid,&returned_status) != 0){
//         fprintf(2,"[ERROR] kthread_join\n");
//         return -1;
//     }
//     int tid2 = kthread_create(fib100,memory2,4000);
//     fprintf(2,"[main] created %d\n",tid2);
//     kthread_join(tid2,&returned_status);
//     if(returned_status != 1){
//         fprintf(2,"[ERROR] wrong status\n");
//         return -1;   
//     }
//     printf("SUCCESS!\n");
//     return 1;
// }

// more tests
void* f1() {
    fork();
    fork();
    printf("0");
    kthread_exit(0);
    return 0;
}

void* f2() {
    printf("0");
    sleep(1);
    exit(0);
     return 0;
}

void* f3() {
    if (fork()) {
        wait(0);
        printf("0");
        kthread_exit(0);
    }
    else {
        sleep(5);
        exit(0);
    }
     return 0;
}

void* f4() {
    int pid = fork();
    if (pid) {
        kill(pid);
        wait(0);
        printf("0");
        kthread_exit(0);
    }
    else {
        sleep(1);
        printf("1");
    }
     return 0;
}

void* f5() {
    char *argv[] = {"ls", 0};
    exec("ls", argv);
    printf("0");
     return 0;
}

void* f6() {
    char *argv[] = {"asdf", 0};
    exec("asdf", argv);
    printf("0");
    kthread_exit(0);
     return 0;
}

void* f7() {
    int pid = fork();
    if (pid) {
        kill(pid);
        sleep(5);
        wait(0);
        printf("0");
        kthread_exit(0);
    }
    else {
        sleep(1000);

    }
     return 0;
}

void* f8() {
    if (kthread_id() == 2) {
        kthread_kill(3);
        kthread_join(3, 0);
        printf("2");
        kthread_exit(0);
    }
    else {
        kthread_kill(2);
        kthread_join(2, 0);
        printf("3");
        kthread_exit(0);
    }
     return 0;
}

void* f9() {
    if (kthread_id() == 2) {
        kthread_kill(3);
        kthread_join(3, 0);
        printf("2");
        kthread_exit(0);
    }
    else {
        sleep(1000);
        kthread_kill(2);
        kthread_join(2, 0);
        printf("3");
        kthread_exit(0);
    }
    return 0;
}

//void (*ptr)() = f9;

void create() {
    uint64 stack1 = (uint64)malloc(MAX_STACK_SIZE);
    uint64 stack2 = (uint64)malloc(MAX_STACK_SIZE);
    kthread_create(f9, stack1, MAX_STACK_SIZE);
    kthread_create(f9, stack2, MAX_STACK_SIZE);
    sleep(10);
    printf("\n");
    exit(0);
}

int main() {;
    create();
    exit(0);
}