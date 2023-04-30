#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct proc proc[NPROC];

extern void forkret(void); //task2.2

//int nextktid = 1;
//struct spinlock ktid_lock;


//task2.2
void kthreadinit(struct proc *p){
  initlock(&p->ktid_lock, "nextktid");
  for (struct kthread *kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    initlock(&kt->ktLock, "thread");
    kt->ktState = UNUSED;
    kt->myprocess = p;
    // WARNING: Don't change this line!
    // get the pointer to the kernel stack of the kthread
    kt->kstack = KSTACK((int)((p - proc) * NKT + (kt - p->kthread)));
  }
}

struct kthread *mykthread(){ 
  push_off();
  struct cpu *c = mycpu();
  struct kthread *kt = c->kthread;
  pop_off();
  return kt;
}

int allocktid(struct proc *p){
  int ktid;
  acquire(&p->ktid_lock);
  ktid = p->ktidCounter;
  p->ktidCounter = p->ktidCounter + 1;
  release(&p->ktid_lock);
  return ktid;
}

struct trapframe *get_kthread_trapframe(struct proc *p, struct kthread *kt)
{
  return p->base_trapframes + ((int)(kt - p->kthread));
}


struct kthread* allockthread(struct proc *p){
  struct kthread *kt;
  int found = 0;
  for (kt = p->kthread; kt < &p->kthread[NKT] && !found; kt++){
    acquire(&kt->ktLock);
    if(kt->ktState == UNUSED){
      found = 1;
      break;
    }
    else {
      release(&kt->ktLock);
    }
  }
  if(!found){
    return 0;
  }
  else{
    kt->ktId = allocktid(p);
    kt->ktState = USED;
    kt->myprocess = p;
    if((kt->trapframe = get_kthread_trapframe(p,kt)) == 0){
      freekthread(kt);
      release(&kt->ktLock);
      return 0;
    }
    memset(&kt->ktContext, 0, sizeof(kt->ktContext));
    kt->ktContext.ra = (uint64)forkret;
    kt->ktContext.sp = kt->kstack + PGSIZE;   //what we need to add insted of PGSIZE ? TODO
    return kt;
  }
}

void freekthread(struct kthread *kt){
  kt->trapframe = 0;
  kt->ktChan = 0;
  kt->ktKilled = 0;
  kt->ktXstate = 0;
  kt->ktId = 0;
  kt->myprocess = 0;

  kt->ktState = UNUSED;
}
