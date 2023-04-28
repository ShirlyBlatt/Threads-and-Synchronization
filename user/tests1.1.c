#include "../kernel/types.h"
#include "user.h"
#include "../kernel/stat.h"
#include "uthread.h"

void hello1(){
    printf("Hello World from hello1");
    uthread_exit();
}

void hello2(){
    printf("Hello World from hello2");
    uthread_exit();
}

int main(int argc, char *argv[]){
    printf("Hello World from main\n");    
    uthread_create(&hello1, LOW);
    uthread_create(&hello2, HIGH);
    uthread_start_all();
    printf("after all\n");
    return 0;
}