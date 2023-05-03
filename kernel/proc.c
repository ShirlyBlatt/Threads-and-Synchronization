#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

struct kthread *initkthread;

int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[]; // trampoline.S 


//task2.3
//int firstThreadToExit = 1;
// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void
proc_mapstacks(pagetable_t kpgtbl)
{
  struct proc *p;
  
  for(p = proc; p < &proc[NPROC]; p++) {
    for (struct kthread *kt = p->kthread; kt < &p->kthread[NKT]; kt++) {
      char *pa = kalloc();
      if(pa == 0)
        panic("kalloc");
      uint64 va = KSTACK((int) ((p - proc) * NKT + (kt - p->kthread)));
      kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
    }
  }
}

// initialize the proc table.
void
procinit(void)
{
  struct proc *p;
  
  initlock(&pid_lock, "nextpid");
  initlock(&wait_lock, "wait_lock");
  for(p = proc; p < &proc[NPROC]; p++) {
      initlock(&p->lock, "proc");
      p->state = UNUSED;
      kthreadinit(p);
  }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int
cpuid()
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu*
mycpu(void)
{
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc*
myproc(void)
{
  push_off();
  struct cpu *c = mycpu();
  //task2.2
 // struct kthread *kt = mykthread();
  struct kthread *kt = c->kthread;
  if(kt == 0){
    return 0;
  }
  struct proc *p = kt->myprocess;
  pop_off();
  return p;
}

int
allocpid()
{
  int pid;
  
  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();
  p->state = USED;

  // Allocate a trapframe page.
  if((p->base_trapframes = (struct trapframe *)kalloc()) == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if(p->pagetable == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }
  acquire(&p->ktid_lock);  //task2.2
  p->ktidCounter = 1; //task2.2
  release(&p->ktid_lock);
  allockthread(p);    //task2.2


  // Set up new context to start executing at forkret,
  // which returns to user space.
  // memset(&p->context, 0, sizeof(p->context));
  // p->context.ra = (uint64)forkret;
  // p->context.sp = p->kstack + PGSIZE;

  return p;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void
freeproc(struct proc *p)
{
  if(p->base_trapframes)
    kfree((void*)p->base_trapframes);
  p->base_trapframes = 0;
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  //task2.2
  

  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->killed = 0;
  p->xstate = 0;

  struct kthread *kt;
  for (kt= p->kthread; kt < &p->kthread[NKT]; kt++){ //task2.2
    freekthread(kt);
  }

  p->ktidCounter = 0;
  p->state = UNUSED;
 
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if(pagetable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if(mappages(pagetable, TRAMPOLINE, PGSIZE,
              (uint64)trampoline, PTE_R | PTE_X) < 0){
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
  if(mappages(pagetable, TRAPFRAME(0), PGSIZE,
              (uint64)(p->base_trapframes), PTE_R | PTE_W) < 0){
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void
proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME(0), 1, 0);
  uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// assembled from ../user/initcode.S
// od -t xC ../user/initcode
uchar initcode[] = {
  0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
  0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
  0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
  0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
  0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
  0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

// Set up first user process.
void
userinit(void)
{
  struct proc *p;

  p = allocproc();
  initproc = p;
  
  // allocate one user page and copy initcode's instructions
  // and data into it.
  uvmfirst(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->kthread[0].trapframe->epc = 0;      // user program counter
  p->kthread[0].trapframe->sp = PGSIZE;  // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->kthread[0].ktState = RUNNABLE; //task2.2

  release(&p->kthread[0].ktLock);     //task2.2
  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint64 sz;
  struct proc *p = myproc();

  sz = p->sz;
  if(n > 0){
    if((sz = uvmalloc(p->pagetable, sz, sz + n, PTE_W)) == 0) {
      return -1;
    }
  } else if(n < 0){
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();  
  struct kthread *kt = mykthread();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy user memory from parent to child.
  if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  //task2.2
  if(&(np->kthread[0]) == 0){
    freeproc(np);
    release(&np->lock); 
    return -1;
  }

  // copy saved user registers.
  *(np->kthread[0].trapframe) = *(kt->trapframe);

  // Cause fork to return 0 in the child.
  np->kthread[0].trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->kthread[0].ktLock);    //task2.2
  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);                 //maybe we dont need ? //TODO //may
  acquire(&np->kthread[0].ktLock);    //task2.2
  np->kthread[0].ktState = RUNNABLE;  //task2.2
  release(&np->kthread[0].ktLock);    //task2.2
  release(&np->lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void
reparent(struct proc *p)
{
  struct proc *pp;

  for(pp = proc; pp < &proc[NPROC]; pp++){
    if(pp->parent == p){
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void
exit(int status)
{
  //static int firstThreadToExit = 1;
  struct proc *p = myproc();
  //acquire(&p->lock);
  
 // if(firstThreadToExit){
  //  firstThreadToExit = 0;
  //  release(&p->lock);
    terminate_all_other_kthreads();  //task2.3
  
  // }
  //   else{
  //   release(&p->lock);
  // }
    if(p == initproc)
      panic("init exiting");

    // Close all open files.
    for(int fd = 0; fd < NOFILE; fd++){
      if(p->ofile[fd]){
        struct file *f = p->ofile[fd];
        fileclose(f);
        p->ofile[fd] = 0;
      }
    }

    begin_op();
    iput(p->cwd);
    end_op();
    p->cwd = 0;
  // }
  // else{
  //   release(&p->lock);
  // }
    acquire(&wait_lock);

    // Give any children to init.
    reparent(p);

    // Parent might be sleeping in wait().
    wakeup(p->parent);
    
    acquire(&p->lock);

    p->xstate = status;
    p->state = ZOMBIE;

    release(&p->lock);
    //task 2.2
    // struct kthread *kt;
    // for (kt = p->kthread; kt < &p->kthread[NKT]; kt++) {
    //   acquire(&kt->ktLock);
    //   kt->ktState = ZOMBIE;
    //   release(&kt->ktLock);
    // 
    //struct kthread *kt = mykthread();
    acquire(&mykthread()->ktLock);
    mykthread()->ktState = ZOMBIE;
    release(&mykthread()->ktLock);

    //acquire(&mykthread()->ktLock);  //task 2.2
    

    release(&wait_lock);
    acquire(&mykthread()->ktLock);
  // }
  // else{
  //   release(&p->lock);
  //   kthread_exit(status);
  // }
  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}



// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(uint64 addr)
{
  struct proc *pp;
  int havekids, pid;
  struct proc *p = myproc();


  acquire(&wait_lock);

  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(pp = proc; pp < &proc[NPROC]; pp++){
      if(pp->parent == p){
        // make sure the child isn't still in exit() or swtch().
        acquire(&pp->lock);                                  
        havekids = 1;
        if(pp->state == ZOMBIE){
          // Found one.
          pid = pp->pid;
          if(addr != 0 && copyout(p->pagetable, addr, (char *)&pp->xstate,
                                  sizeof(pp->xstate)) < 0) {
            release(&pp->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(pp);
          release(&pp->lock);
          release(&wait_lock);
          return pid;
        }
        release(&pp->lock);
      }
    }

    //task2.2
    // struct kthread* me = mykthread();
    // acquire(&me->ktLock);
    // No point waiting if we don't have any children.
    if(!havekids || killed(p) || kthread_get_killed(mykthread())){
      //release(&me->ktLock);
      release(&wait_lock);
      return -1;
    }
    //task2.2
    //release(&me->ktLock);
    
    // Wait for a child to exit.
    sleep(p, &wait_lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();

 //task2.2 
  c->kthread = 0;
  for(;;){
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();

    for(p = proc; p < &proc[NPROC]; p++) {
       if(p->state == USED) { //may
        //task2.2
        struct kthread *kt;
        for (kt = p->kthread; kt < &p->kthread[NKT]; kt++){
          acquire(&kt->ktLock);
          if(kt->ktState == RUNNABLE){
            kt->ktState = RUNNING;
            c->kthread = kt;
            swtch(&c->context, &kt->ktContext);
            c->kthread = 0;
          }
          release(&kt->ktLock);
        }
       }
    }
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct kthread *kt = mykthread();

  if(!holding(&kt->ktLock))          //task2.2
    panic("sched kt->ktLock");
  if(mycpu()->noff != 1)
    panic("sched locks");
  if(kt->ktState == RUNNING)      //task2.2
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&kt->ktContext, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  //task2.2
  struct kthread *kt = mykthread();
  acquire(&kt->ktLock);
  kt->ktState = RUNNABLE;
  sched();
  release(&kt->ktLock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void
forkret(void)
{
  static int first = 1;

  // Still holding p->lock from scheduler.
  release(&mykthread()->ktLock);      //task2.2

  if (first) {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    first = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{

  struct kthread *kt = mykthread(); //task2.2
  
  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

   //DOC: sleeplock1
  acquire(&kt->ktLock); //task2.2
  release(lk);

  // Go to sleep.


  kt->ktChan = chan;         //task2.2
  kt->ktState = SLEEPING; //task2.2

  sched();

  // Tidy up.
  kt->ktChan = 0;           //task2.2

  // Reacquire original lock.
  release(&kt->ktLock);     //task2.2
  acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void
wakeup(void *chan)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
      //acquire(&p->lock); //TODO
      struct kthread *kt = 0;
      for (kt = p->kthread; kt < &p->kthread[NKT]; kt++){ //task2.2
          if(kt != mykthread() && kt != 0){
            acquire(&kt->ktLock);
            if(kt->ktState == SLEEPING && kt->ktChan == chan){
              kt->ktState = RUNNABLE;
            }
            release(&kt->ktLock);
          }
      }
      //release(&p->lock);
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int
kill(int pid)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      p->killed = 1;  //TODO where to release?
      release(&p->lock);
      //task2.2
      struct kthread *kt;
      for (kt= p->kthread; kt < &p->kthread[NKT]; kt++){
          acquire(&kt->ktLock);
          kt->ktKilled = 1;
          if(kt->ktState == SLEEPING){
            kt->ktState = RUNNABLE;
          }
          release(&kt->ktLock);
      }
      //release(&p->lock);
      return 0;
    }
    else{
      release(&p->lock);
    }
    
  }
  return -1;
}

void
setkilled(struct proc *p)
{
  acquire(&p->lock);
  p->killed = 1;
  release(&p->lock);
}

int
killed(struct proc *p)
{
  int k;
  
  acquire(&p->lock);
  k = p->killed;
  release(&p->lock);
  return k;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int
either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if(user_dst){
    return copyout(p->pagetable, dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int
either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if(user_src){
    return copyin(p->pagetable, dst, src, len);
  } else {
    memmove(dst, (char*)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [USED]      "used",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;

  printf("\n");
  for(p = proc; p < &proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
  }
}

//task2.3
int kthread_create(uint64 start_func,uint64 stack, int stack_size){
  int tid;
  struct proc *p = myproc();
  //cquire(&p->lock);
  struct kthread *kt = allockthread(p);
  if(kt == 0){
    //release(&p->lock);
    return -1;
  }
  else{
    kt->ktState = RUNNABLE;
    kt->trapframe->epc = (uint64)start_func;
    kt->trapframe->sp = (uint64)(stack + stack_size);
    tid = kt->ktId;
    release(&kt->ktLock);
    //release(&p->lock);
    return tid;
  }
}

//task2.3
int kthread_id(void){
  struct kthread *kt = mykthread();
  int ktid;
  acquire(&kt->ktLock);
  ktid = kt->ktId;
  release(&kt->ktLock);
  return ktid;
}

//task2.3
int kthread_kill(int ktid){
  struct proc *p = myproc();
  struct kthread *kt;
  int found = 0;
  //acquire(&p->lock);  //TODO
  for (kt = p->kthread; kt < &p->kthread[NKT] && (found == 0); kt++){
    acquire(&kt->ktLock);
    if(kt->ktId == ktid){
      found = 1;
      break;
      }
    else{
      release(&kt->ktLock);
    }
  }
if(found == 0){
  //release(&p->lock);
  return -1;
}
else{
  kt->ktKilled = 1;
  if(kt->ktState == SLEEPING){
    kt->ktState = RUNNABLE;
  }
  release(&kt->ktLock);
  //release(&p->lock);
  return 0;
  }
}

//task2.3
void kthread_exit(int status){
  struct kthread *kt = mykthread();
  struct kthread *temp;
  struct proc *p = myproc();
  int found = 0;
  acquire(&p->lock); //TODO
  for (temp = p->kthread; temp < &p->kthread[NKT] ; temp++){
    acquire(&temp->ktLock);
    if((temp->ktState != UNUSED) && (temp->ktState != ZOMBIE) && (temp!=kt)){
      found = 1;
      release(&temp->ktLock);
      break;
    }
    release(&temp->ktLock);
  }
  release(&p->lock);

  if(found == 0){
    exit(status);
  }
  
  else{
    acquire(&p->lock);    //TODO
    
    wakeup(mykthread());

    release(&p->lock);
    
    acquire(&kt->ktLock);
    kt->ktXstate = status;     
    kt->ktState = ZOMBIE;
   
    //release(&wait_lock);
  
    sched();
    panic("zombie exit");
  }

}



//task2.3
struct kthread* get_kthread_by_ktid(int ktid){
  struct proc *p = myproc();
  struct kthread *kt;
  //acquire(&p->lock); //TODO
  for (kt = p->kthread; kt < &p->kthread[NKT] ; kt++){
    acquire(&kt->ktLock);
    if(kt->ktId == ktid){
      release(&kt->ktLock);
      //release(&p->lock);
      return kt;
    }
    else{
      release(&kt->ktLock);
    }
  }
  //release(&p->lock);
  return 0;
}

//task2.3
int kthread_join(int ktid, uint64 status){
  //acquire(&wait_lock);
  struct proc *p = myproc();
  acquire(&p->lock);

  for(;;){

    struct kthread *kt = get_kthread_by_ktid(ktid);
    if(kt){
      acquire(&kt->ktLock);
      if(kt->ktState == ZOMBIE){
          if(status != 0 && copyout(myproc()->pagetable, status, (char *)&kt->ktXstate,
                                    sizeof(kt->ktXstate)) < 0) {
            //we couldnt copy
            release(&kt->ktLock);
             release(&p->lock);
            //release(&wait_lock);
            return -1;
            }
            //we copyied well
            freekthread(kt); 
            release(&kt->ktLock);
             release(&p->lock);
            //release(&wait_lock);
            return 0;
      }
      //the ktid thread didnt terminate
      release(&kt->ktLock);
      
      //sleep(kt,&wait_lock);
    }
    if(kt == 0 || kthread_get_killed(mykthread())){  //didnt found the thread with ktid
       release(&p->lock);
      //release(&wait_lock);
      return -1;
    } 
    sleep(kt,&p->lock);
  }
  return -1;
}


//task2.3
int kthread_get_killed( struct kthread *kt){
  int ktkilled;
  acquire(&kt->ktLock);
  ktkilled = kt->ktKilled;
  release(&kt->ktLock);
  return ktkilled;
}

void terminate_all_other_kthreads(void){
  struct proc *p = myproc();
  
  int temp_ktid = 0;
  while(1){
    //acquire(&p->lock);  //TODO
    temp_ktid = 0;
    for(struct kthread *temp = p->kthread; temp < &p->kthread[NKT] ; temp++){
      if((mykthread() != temp) && (temp->ktState != UNUSED) && (temp->ktState != ZOMBIE) && temp!=0){
        acquire(&temp->ktLock);
        temp_ktid = temp->ktId;
        release(&temp->ktLock);
        break;
      }
   }
    //release(&p->lock);
    if(temp_ktid){
      kthread_kill(temp_ktid);
      kthread_join(temp_ktid,0);
    }
     else{
        break;
     }
  }
}