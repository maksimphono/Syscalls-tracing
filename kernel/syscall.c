#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if(copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);
extern uint64 sys_etrace(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_etrace]  sys_etrace
};


typedef enum {
    ___NONE_TYPE = 0, 
    INT_32_TYPE  = 1, 
    UINT_32_TYPE = 2, 
    INT_16_TYPE  = 3, 
    UINT_16_TYPE = 4, 
    STRING_TYPE  = 5, 
    ADDRESS_TYPE = 6, 
    OTHER_TYPE   = 7
} Syscall_arg_type;

static Syscall_arg_type Syscall_arg_types_LUT[][6] = {
[SYS_fork]    { ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_exit]    { INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_wait]    { ADDRESS_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_pipe]    { ADDRESS_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_read]    { INT_32_TYPE  , ADDRESS_TYPE , INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_kill]    { INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_exec]    { STRING_TYPE  , ADDRESS_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_fstat]   { INT_32_TYPE  , ADDRESS_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_chdir]   { STRING_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_dup]     { INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_getpid]  { ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_sbrk]    { INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_sleep]   { INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_uptime]  { ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_open]    { STRING_TYPE  , INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_write]   { INT_32_TYPE  , ADDRESS_TYPE , INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_mknod]   { STRING_TYPE  , INT_16_TYPE  , INT_16_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_unlink]  { STRING_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_link]    { STRING_TYPE  , STRING_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_mkdir]   { STRING_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_close]   { INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  },
[SYS_etrace]  { STRING_TYPE  , INT_32_TYPE  , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE , ___NONE_TYPE  }
};

static const char* Syscalls_names[NSYSCALLS + 1] = {"", "fork", "exit", "wait", "pipe", "read", "kill", "exec", "fstat", "chdir", "dup", "getpid", "sbrk", "sleep", "uptime", "open", "write", "mknod", "unlink", "link", "mkdir", "close", "etrace"};

const char* get_syscall_name(int syscall_num) {
    //const char* names[] = {"", "fork", "exit", "wait", "pipe", "read", "kill", "exec", "fstat", "chdir", "dup", "getpid", "sbrk", "sleep", "uptime", "open", "write", "mknod", "unlink", "link", "mkdir", "close", "etrace"};
    return Syscalls_names[syscall_num];
} 

const Syscall_arg_type* get_syscall_argument_types(int syscall_num) {
    return Syscall_arg_types_LUT[syscall_num];
}

uint8 collect_syscall_arguments(char str_arguments[6][MAX_ARG_LEN], int syscall_num, struct trapframe* trapframe) {
    Syscall_arg_type* types = Syscall_arg_types_LUT[syscall_num];
    uint64 raw_arguments[6] = {trapframe->a0, trapframe->a1, trapframe->a2, trapframe->a3, trapframe->a4, trapframe->a5};
    char buffer[MAX_STR_P] = {};
    const char* syscall_args_print_formats[] = {0x0, "%d", "%u", "%d", "%u", "\"%s\"", "0x%lx", "0x%lx"};

    uint8 i = 0;

    // collect arguments in form of arrays of strings
    for (i = 0; types[i] != ___NONE_TYPE && i < 6; i++) {
        memset(buffer, '\0', MAX_STR_P);

        switch(types[i]) {
        case INT_32_TYPE:
        case UINT_32_TYPE:
        case INT_16_TYPE:
        case UINT_16_TYPE:
        case ADDRESS_TYPE:
        case OTHER_TYPE:{
            sprintf(str_arguments[i], MAX_ARG_LEN, (char*)(syscall_args_print_formats[types[i]]), raw_arguments[i]);
            break;
        }
        case STRING_TYPE:{
            int len = argstr(i, buffer, MAX_ARG_LEN);
            //len -= 1;
            sprintf(str_arguments[i], MAX_STR_P + 2, (char*)(syscall_args_print_formats[types[i]]), buffer);
            if (len > MAX_STR_P) {
              sprintf(&str_arguments[i][MAX_STR_P + 1], 4, "\"...");
            }
            break;
        }
        default:
          continue;
        }
    }

    return i;
}

void print_traced_syscall(int pid, int syscall_num, char syscall_arguments[6][MAX_ARG_LEN], uint8 arguments_len, uint64 ret) {
  printf("%d: syscall %s(", pid, get_syscall_name(syscall_num));
  if (arguments_len > 0) {
    for (uint8 i = 0; i < arguments_len - 1; i++) {
      printf("%s, ", syscall_arguments[i]);
    }
    printf("%s", syscall_arguments[arguments_len - 1]);
  }
  printf(")");
  if (syscall_num == SYS_exit)
    printf(" -> ?\n");
  else
    printf(" -> %ld\n", (long)ret);
}



void
syscall(void)
{
  int num, pid = 0;
  struct proc *p = myproc();
  char syscall_arguments[6][MAX_ARG_LEN] = {{}, {}, {}, {}, {}, {}};
  uint8 syscall_arguments_len = 0;

  num = p->trapframe->a7;

  //printf("\n\n%ld\n\n", p->trace_mask);
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0
    if (p->is_traced == 1 && p->trace_mask & (1L << num)) {
      pid = p->pid;
      syscall_arguments_len = collect_syscall_arguments(syscall_arguments, num, p->trapframe);
      if (num == SYS_exit) {
        print_traced_syscall(pid, num, syscall_arguments, syscall_arguments_len, 0);
      }
      p->trapframe->a0 = syscalls[num]();
      print_traced_syscall(pid, num, syscall_arguments, syscall_arguments_len, p->trapframe->a0);
    } else {
      p->trapframe->a0 = syscalls[num]();
    }
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
