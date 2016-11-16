#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{ 
  int *sys_call = f->esp;
  printf ("System call!!! Number = %d\n", *sys_call);
  
  switch(*sys_call)
  {
  	case SYS_HALT:
  		halt();
  		break;
  	case SYS_EXIT:
  		// TODO
  		break;
  	case SYS_EXEC:
  		// TODO
  		break;
  	case SYS_WAIT:
  		// TODO
  		break;
  	case SYS_CREATE:
  		// TODO
  		break;
  	case SYS_REMOVE:
  		// TODO
  		break;
  	case SYS_OPEN:
  		// TODO
  		break;
  	case SYS_FILESIZE:
  		// TODO
  		break;
  	case SYS_READ:
  		// TODO
  		break;
  	case SYS_WRITE:
  		// TODO
  		break;
  	case SYS_SEEK:
  		// TODO
  		break;
  	case SYS_TELL:
  		// TODO
  		break;
  	case SYS_CLOSE:
  		// TODO
  		break;
  	default:
  		break;
  }

  thread_exit (); // FIXME: Remove this after implementating some system calls
}

void halt(void)
{
	shutdown_power_off();
}

void exit(int status)
{
	// TODO
}

pid_t exec(const char* cmd_line)
{
	// TODO
}

int wait(pid_t pid)
{
	// TODO
}

bool create(const char* file, unsigned initial_size)
{
	// TODO
}

bool remove(const char* file)
{
	// TODO
}

int open(const char* file)
{
	// TODO
}

int filesize(int fd)
{
	// TODO
}

int read(int fd, void* buffer, unsigned size)
{
	// TODO
}

int write(int fd, const void* buffer, unsigned size)
{
	// TODO
}

void seek(int fd, unsigned position)
{
	// TODO
}

unsigned tell(int fd)
{
	// TODO
}

void close(int fd)
{
	// TODO
}
