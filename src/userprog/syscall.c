#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "lib/kernel/console.h"
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
  		// TODO
  		break;
  	case SYS_WAIT: // 3
  		// TODO
  		break;
  	case SYS_CREATE: // 4
  		// TODO
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
  char proc_name[20];
  char* tname = thread_name();
  int i = 0;
  while(tname[i] != ' ' && tname[i] != '\0')
  {
    proc_name[i] = tname[i];
    i++;
  }
  proc_name[i] = '\0';

  printf("%s: exit(%d)\n", proc_name, status);
  thread_exit();
}

pid_t exec(const char* cmd_line UNUSED)
{
	// TODO
  return -1;
}

int wait(pid_t pid UNUSED)
{
	// TODO
  return -1;
}

bool create(const char* file, unsigned initial_size)
{
	// new code here
  if (!file) 
  {
    return false;
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
