#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/user/syscall.h"
#include "userprog/process.h"

struct fd_process {
  int fd;
  pid_t pid;
  struct file* file;
  struct list_elem fd_elem;
};

void syscall_init (void);

void halt(void);
void exit_(int status);
pid_t exec_(const char* cmd_line);
int wait_(pid_t pid);

bool create_(const char* file, unsigned initial_size);
bool remove_(const char* file);
int open_(const char* file);
int filesize_(int fd);
int read_(int fd, void* buffer, unsigned size);
int write(int fd, const void* buffer, unsigned size);
void seek_(int fd, unsigned position);
unsigned tell_(int fd);
void close_(int fd);

#endif /* userprog/syscall.h */
