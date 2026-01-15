#ifndef _TRACE_H_
#define _TRACE_H_

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

extern const Syscall_arg_type Syscall_arg_types_LUT[][SYS_MAXARG];

extern const char* Syscalls_names[NSYSCALLS];

const char* get_syscall_name(int syscall_num);

const Syscall_arg_type* get_syscall_argument_types(int syscall_num);

uint8 collect_syscall_arguments(char str_arguments[SYS_MAXARG][MAX_ARG_LEN], int syscall_num, struct trapframe* trapframe);

void print_traced_syscall(int pid, int syscall_num, char syscall_arguments[SYS_MAXARG][MAX_ARG_LEN], uint8 arguments_len, uint64 ret);

#endif