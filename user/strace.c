#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int parse_param(const char *arg, const char *name, const char *abb, char *value, int value_size) {
    int name_len = strlen(name);
    int abb_len = strlen(abb);
    char *arg_name = malloc(name_len + 1);
    char *arg_abb = malloc(abb_len + 1);
    int ret = 0;
    for (int i = 0; i < name_len; i++) {
      arg_name[i] = arg[i];
    }
    for (int i = 0; i < abb_len; i++) {
      arg_abb[i] = arg[i];
    }
    if (strcmp(arg_name, name) == 0) {
      if (arg[name_len] == '=') {
        strcpy(value, arg + name_len + 1);
        value[value_size - 1] = '\0';
      }
      ret = 1;
    } else if (strcmp(arg_abb, abb) == 0) {
      if (arg[abb_len] == '=') {
        strcpy(value, arg + abb_len + 1);
        value[value_size - 1] = '\0';
      }
      ret = 1;
    }
    free(arg_name); arg_name = 0;
    free(arg_abb); arg_abb = 0;
    return ret;
}

int
strncmp(const char *p, const char *q, uint n)
{
  while(n > 0 && *p && *p == *q)
    n--, p++, q++;
  if(n == 0)
    return 0;
  return (uchar)*p - (uchar)*q;
}


int get_syscall_num(char* name) {
  const char* syscalls_names[23] = {"", "fork", "exit", "wait", "pipe", "read", "kill", "exec", "fstat", "chdir", "dup", "getpid", "sbrk", "sleep", "uptime", "open", "write", "mknod", "unlink", "link", "mkdir", "close", "etrace"};
  uint length[23] = {0, 4, 4, 4, 4, 4, 4, 4, 5, 5, 3, 6, 4, 5, 6, 4, 5, 5, 6, 4, 5, 5, 6};
  for (int i = 1; i < 23; i++) {
    if (strncmp(syscalls_names[i], name, length[i]) == 0){
      fprintf(2, "%s\n", syscalls_names[i]);
      return i;
    }
  }
  return 0;
}

uint64 get_syscalls_mask(char* syscalls) {
  uint64 mask = 0x0;
  int start = 0, end = 0;
  
  
  // TODO: implement logic of getting syscalls mask here
  while (1) {
    if (syscalls[end] == ',' || syscalls[end] == '\0') {
      //printf("%s", &syscalls[start]);
      mask |= 1 << get_syscall_num(&syscalls[start]);
      start = end + 1;
      if (syscalls[end] == '\0') break;
    }
    end += 1;
  }

  return mask;
}


int
main(int argc, char *argv[])
{
  int i;
  char *nargv[MAXARG];
  char *help = "Usage: strace [--trace|-t=all|SYSCALL] [--follow-forks|-f] command\n";

  if (argc < 2) {
    fprintf(2, "%s", help);
    exit(1);
  }

  char syscall_name[MAX_ARG_LEN] = {0};
  int follow_forks = 0;
  char buffer[MAX_ARG_LEN] = {0};

  for (i = 1; i < argc && i < MAXARG; i++) {
    if (parse_param(argv[i], "--trace", "-t", buffer, sizeof(buffer))) {
      strcpy(syscall_name, buffer);
    } else if (parse_param(argv[i], "--follow-forks", "-f", buffer, sizeof(buffer))) {
      follow_forks = 1;
    } else {
      break;
    }
  }

  if (i == argc) {
    fprintf(2, "%s", help);
    exit(1);
  }

  int ret = 0;
  if (syscall_name[0]) {
    ret = etrace(get_syscalls_mask("read,write,open,etrace,fork,pipe,mkdir"), follow_forks); //syscall_name
  } else {
    ret = etrace(get_syscalls_mask("read,write,open,etrace,fork,pipe,mkdir"), follow_forks);
  }
  if (ret < 0) {
    fprintf(2, "%s: etrace failed\n", argv[0]);
    exit(1);
  }
  
  for(int j=i; j < argc && j < MAXARG; j++){
    nargv[j-i] = argv[j];
  }

  if (fork() == 0) {
    // Child process
    int r = exec(nargv[0], nargv);
    exit(r);
  } else {
    wait(&ret);
  }

  if (ret < 0) {
    fprintf(2, "exec %s failed\n", nargv[0]);
    exit(ret);
  }
  exit(0);
}
