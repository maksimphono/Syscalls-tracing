#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "trace.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
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
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
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
  return kill(pid);
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

//static const char* Syscalls_names[NSYSCALLS] = {"", "fork", "exit", "wait", "pipe", "read", "kill", "exec", "fstat", "chdir", "dup", "getpid", "sbrk", "sleep", "uptime", "open", "write", "mknod", "unlink", "link", "mkdir", "close", "etrace"};

uint64
sys_etrace(void)
{
  // TODO: Implement syscall etrace here
  char raw_syscalls_names[MAX_ARG_LEN] = {};
  int trace_fork = 0;
  uint64 trace_all = 0;
  argaddr(0, &trace_all);

  if (trace_all == 0L) {
    myproc()->trace_mask = 0xfffffffffffffffeUL;
  } else {
    if (argstr(0, raw_syscalls_names, MAX_ARG_LEN) < 0) return -1;
    myproc()->trace_mask = get_syscalls_mask(raw_syscalls_names);
  }

  myproc()->is_traced = 1;
  argint(1, &trace_fork);
  myproc()->trace_fork = trace_fork;

  //printf("Mask: %ld", myproc()->trace_mask);
  if (myproc()->trace_mask & 0x1) return -1;
  return 0;//get_syscalls_mask((char*)p);
}
