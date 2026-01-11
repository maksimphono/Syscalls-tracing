#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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

int get_syscall_num(char* name) {
  const char* syscalls_names[23] = {"", "fork", "exit", "wait", "pipe", "read", "kill", "exec", "fstat", "chdir", "dup", "getpid", "sbrk", "sleep", "uptime", "open", "write", "mknod", "unlink", "link", "mkdir", "close", "etrace"};
  uint length[23] = {0, 4, 4, 4, 4, 4, 4, 4, 5, 5, 3, 6, 4, 5, 6, 4, 5, 5, 6, 4, 5, 5, 6};
  for (int i = 1; i < 23; i++) {
    if (strncmp(syscalls_names[i], name, length[i]) == 0){
      //printf("%s, %d\n", syscalls_names[i], i);
      return i;
    }
  }
  return 0;
}

uint64 get_syscalls_mask(char* raw_syscalls_names) {
  uint64 mask = 0x0;
  int start = 0, end = 0;
  int num = 0;

  while (1) {
    if (raw_syscalls_names[end] == ',' || raw_syscalls_names[end] == '\0') {
      //printf("%s", &raw_syscalls_names[start]);
      num = get_syscall_num(&raw_syscalls_names[start]);
      if (num != 0) {
        mask |= 1 << num;
      }
      start = end + 1;
      if (raw_syscalls_names[end] == '\0') break;
    }
    end += 1;
  } 

  return mask;
}

uint64
sys_etrace(void)
{
  // TODO: Implement syscall etrace here
  char raw_syscalls_names[128] = {};
  int trace_fork = 0;
  argstr(0, raw_syscalls_names, 128);
  argint(1, &trace_fork);
  myproc()->is_traced = 1;
  myproc()->trace_fork = trace_fork;
  myproc()->trace_mask = get_syscalls_mask(raw_syscalls_names);
  //printf("Mask: %ld", myproc()->trace_mask);
  if (myproc()->trace_mask & 0x1) return -1;
  return 0;//get_syscalls_mask((char*)p);
}
