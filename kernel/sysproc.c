#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "procinfo.h"
extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_hello(void)
{
  printf("Hello from kernel!\n");
  return 0;
}

uint64
sys_getprocs(void)
{
  uint64 user_addr;   
    int max_procs;

    
    if(argaddr(0, &user_addr) < 0)
        return -1;

    if(argint(1, &max_procs) < 0)
        return -1;

    if(max_procs <= 0)
        return -1;

    struct procinfo pi; 
    int count = 0;   
    for(int i = 0; i < NPROC; i++){
        struct proc *p = &proc[i];

        acquire(&p->lock);
        if(p->state != UNUSED){
            
            pi.pid   = p->pid;
            pi.ppid  = p->parent ? p->parent->pid : 0;
            pi.state = p->state;
            pi.sz    = p->sz;
            safestrcpy(pi.name, p->name, sizeof(pi.name));

            
            if(copyout(myproc()->pagetable,
                       user_addr + count * sizeof(struct procinfo),
                       (char*)&pi,
                       sizeof(pi)) < 0){
                release(&p->lock);
                return -1;
            }

            count++;
        }
        release(&p->lock);

        if(count >= max_procs)
            break;  

         
    }
    return count;

}

