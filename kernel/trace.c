#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"
#include "trace.h"

const Syscall_arg_type Syscall_arg_types_LUT[][SYS_MAXARG] = {
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

const char* Syscalls_names[NSYSCALLS] = {
    "", 
    "fork", 
    "exit", 
    "wait", 
    "pipe", 
    "read", 
    "kill", 
    "exec", 
    "fstat", 
    "chdir", 
    "dup", 
    "getpid", 
    "sbrk", 
    "sleep", 
    "uptime", 
    "open", 
    "write", 
    "mknod", 
    "unlink", 
    "link", 
    "mkdir", 
    "close", 
    "etrace"
};

const char* get_syscall_name(int syscall_num) {
    return Syscalls_names[syscall_num];
} 

const Syscall_arg_type* get_syscall_argument_types(int syscall_num) {
    return Syscall_arg_types_LUT[syscall_num];
}

uint8 collect_syscall_arguments(char str_arguments[SYS_MAXARG][MAX_ARG_LEN], int syscall_num, struct trapframe* trapframe) {
    Syscall_arg_type* types = Syscall_arg_types_LUT[syscall_num];
    uint64 raw_arguments[SYS_MAXARG] = {trapframe->a0, trapframe->a1, trapframe->a2, trapframe->a3, trapframe->a4, trapframe->a5};
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

void print_traced_syscall(int pid, int syscall_num, char syscall_arguments[SYS_MAXARG][MAX_ARG_LEN], uint8 arguments_len, uint64 ret) {
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

