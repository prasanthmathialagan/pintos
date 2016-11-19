#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "lib/kernel/console.h"
#include "lib/string.h"
#include "devices/shutdown.h"

#include "filesys/file.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/*
  Helper method for write system call.
*/
static void write_h (struct intr_frame *f)
{
  void* sp = f->esp;
  
  int fd = *(int*)(sp + 4);
  // printf("fd = %d\n", fd);
  
  uint32_t p = *(uint32_t *)(sp + 8);
  // printf("%x\n", p);
  char* c = (char*) p;
  // printf("%s\n", c);
  
  unsigned size = *(unsigned *)(sp + 12);
  // printf("size = %u\n", size);
  
  int bytes = write (fd, c, size);
  f->eax = bytes;
}

static void exit_h(struct intr_frame *f)
{
  void* sp = f->esp;

  int status = *(int*)(sp + 4);
  f->eax = status;

  exit_(status);
}

static void exec_h(struct intr_frame *f)
{
  void* sp = f->esp;

  uint32_t p = *(uint32_t *)(sp + 4);
  char* args = (char*) p;

  pid_t pid = exec_(args);
  f->eax = pid;
}

static void wait_h(struct intr_frame *f)
{
  void* sp = f->esp;

  pid_t pid = *(pid_t*)(sp + 4);
  int status = wait_(pid);
  f->eax = status;
}

static void create_h(struct intr_frame *f)
{
  void* sp = f->esp;

  uint32_t p = *(uint32_t *)(sp + 4);
  char* file_name = (char*) p;

  unsigned size = *(unsigned *)(sp + 8);
  bool result = create_(file_name, size);
  f->eax = result;
}

static void
syscall_handler (struct intr_frame *f) 
{ 
  int *sys_call = f->esp;
  // printf ("System call!!! Number = %d\n", *sys_call);
  switch(*sys_call)
  {
  	case SYS_HALT: // 0
  		halt();
  		break;
  	case SYS_EXIT: // 1
      exit_h(f);
  		break;
  	case SYS_EXEC: // 2
      exec_h(f);
  		break;
  	case SYS_WAIT: // 3
      wait_h(f);
  		break;
  	case SYS_CREATE: // 4
      create_h(f);
  		break;
  	case SYS_REMOVE: // 5
  		// TODO
  		break;
  	case SYS_OPEN: // 6
  		// TODO
  		break;
  	case SYS_FILESIZE: // 7
  		// TODO
  		break;
  	case SYS_READ: // 8 
  		// TODO
  		break;
  	case SYS_WRITE: // 9
      write_h(f);
  		break;
  	case SYS_SEEK: // 10
  		// TODO
  		break;
  	case SYS_TELL: // 11
  		// TODO
  		break;
  	case SYS_CLOSE: // 12
  		// TODO
  		break;
  	default:
  		break;
  }

  // thread_exit (); // FIXME: Remove this after implementating some system calls
}

void halt(void)
{
	shutdown_power_off();
}

void exit_(int status)
{
  struct process* proc = get_process(thread_current()->tid);
  proc->status = status;
  printf("%s: exit(%d)\n", proc->name, proc->status);
  thread_exit();
}

pid_t exec_(const char* cmd_line)
{
  return process_execute(cmd_line);
}

int wait_(pid_t pid)
{
	return process_wait(pid);
}

bool create_(const char* file, unsigned initial_size)
{
  if (!file || strlen(file) == 0) 
  {
    exit_(-1);
  }

  return filesys_create(file, initial_size);
}

bool remove(const char* file UNUSED)
{
	// TODO
  return false;
}

int open(const char* file UNUSED)
{
	// TODO
  return -1;
}

int filesize(int fd)
{
	// TODO
}

int read(int fd UNUSED, void* buffer UNUSED, unsigned size UNUSED)
{
	// TODO
  return -1;
}

int write(int fd, const void* buffer, unsigned size)
{
  // Writing to console
  if(fd == STDOUT_FILENO)
  {
    putbuf(buffer, size);
    return size;
  }

	// TODO
  return -1;
}

void seek(int fd UNUSED, unsigned position UNUSED)
{
	// TODO
}

unsigned tell(int fd UNUSED)
{
	// TODO
}

void close(int fd UNUSED)
{
	// TODO
}
