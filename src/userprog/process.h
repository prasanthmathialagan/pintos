#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#define MAX_ARGS 25

#include "threads/thread.h"
#include "threads/synch.h"
#include "userprog/syscall.h"

struct process
{
	pid_t pid;
	char name[20];
	bool alive;
	struct lock lock;
	struct condition condition;
	int status;
	// Counter for assigning fd numbers. It must be initialized to 2 (0 and 1 for standard input/output)
	int fd_counter;
	struct list_elem elem;
};

void process_init(void);
struct process* get_process(pid_t pid);
pid_t get_current_pid(void);
int get_and_increment_fd(pid_t pid);

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
