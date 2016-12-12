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
#include "threads/malloc.h"

#include "filesys/file.h"
#include "filesys/filesys.h"
#include "devices/input.h"

static struct list fd_file_mapping;
static struct lock fd_mapping_list_lock;
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  list_init (&fd_file_mapping);
  lock_init(&fd_mapping_list_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void check_access(void* sp)
{
  if(!is_user_vaddr(sp))
    exit_(-1);
}

/*
  Helper method for write system call.
*/
static void write_h (struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);
  check_access(sp + 8);
  check_access(sp + 12);
  
  int fd = *(int*)(sp + 4);
  
  uint32_t p = *(uint32_t *)(sp + 8);
  char* c = (char*) p;
  
  unsigned size = *(unsigned *)(sp + 12);
  
  int bytes = write (fd, c, size);
  f->eax = bytes;
}

static void read_h (struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);
  check_access(sp + 8);
  check_access(sp + 12);
  
  int fd = *(int*)(sp + 4);
  
  uint32_t p = *(uint32_t *)(sp + 8);
  void* buffer = (void*) p;
  
  unsigned size = *(unsigned *)(sp + 12);
  
  int bytes = read_ (fd, buffer, size);
  f->eax = bytes;
}

static void exit_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);

  int status = *(int*)(sp + 4);
  f->eax = status;

  exit_(status);
}

static void exec_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);

  uint32_t p = *(uint32_t *)(sp + 4);
  char* args = (char*) p;

  pid_t pid = exec_(args);
  f->eax = pid;
}

static void wait_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);

  pid_t pid = *(pid_t*)(sp + 4);
  int status = wait_(pid);
  f->eax = status;
}

static void create_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);
  check_access(sp + 8);

  uint32_t p = *(uint32_t *)(sp + 4);
  char* file_name = (char*) p;

  unsigned size = *(unsigned *)(sp + 8);
  bool result = create_(file_name, size);
  f->eax = result;
}

static void remove_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);

  uint32_t p = *(uint32_t *)(sp + 4);
  char* file_name = (char*) p;

  bool result = remove_(file_name);
  f->eax = result;
}

static void open_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);

  uint32_t p = *(uint32_t *)(sp + 4);
  char* file_name = (char*) p;

  int fd = open_(file_name);
  f->eax = fd;
}

static void filesize_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);

  int fd = *(int*)(sp + 4);
  int bytes = filesize_(fd);
  f->eax = bytes;
}

static void seek_h (struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);
  check_access(sp + 8);
  
  int fd = *(int*)(sp + 4);
  unsigned position = *(unsigned *)(sp + 8);
  
  seek_ (fd, position);
}

static void tell_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);

  int fd = *(int*)(sp + 4);
  unsigned position = tell_(fd);
  f->eax = position;
}

static void close_h(struct intr_frame *f)
{
  void* sp = f->esp;
  check_access(sp + 4);

  int fd = *(int*)(sp + 4);
  close_(fd);
}

static void
syscall_handler (struct intr_frame *f) 
{ 
  int *sys_call = f->esp;
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
      remove_h(f);
  		break;
  	case SYS_OPEN: // 6
      open_h(f);
  		break;
  	case SYS_FILESIZE: // 7
      filesize_h(f);
  		break;
  	case SYS_READ: // 8
      read_h(f);
  		break;
  	case SYS_WRITE: // 9
      write_h(f);
  		break;
  	case SYS_SEEK: // 10
      seek_h(f);
  		break;
  	case SYS_TELL: // 11
      tell_h(f);
  		break;
  	case SYS_CLOSE: // 12
      close_h(f);
  		break;
  	default:
  		break;
  }
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
  /*int ret;
  
  if (!cmd_line || !is_user_vaddr(cmd_line))
  {
    return -1;
  }
  lock_acquire(&file_lock);
  ret = process_execute(cmd_line);
  lock_release(&file_lock);
  return ret;*/

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
  lock_acquire(&fd_mapping_list_lock);
  bool result  = filesys_create (file, initial_size);
  lock_release(&fd_mapping_list_lock);
  return result;
}

static struct file* get_file(int fd)
{
  pid_t pid = get_current_pid();
  lock_acquire(&fd_mapping_list_lock);
  struct list_elem *e;
  struct file* file = NULL;
  for (e = list_begin (&fd_file_mapping); e != list_end (&fd_file_mapping); e = list_next (e))
  {
    struct fd_process *fd_process = list_entry (e, struct fd_process, fd_elem);
    if(fd_process->pid == pid && fd_process->fd == fd)
    {
      file = fd_process->file;
      break;
    }
  }
  lock_release(&fd_mapping_list_lock);
  return file;
}

bool remove_(const char* file)
{
  // printf("remove called for filename = %s\n", file);
	// TODO
  if (!file || !is_user_vaddr(file))
  {
    exit_ (-1);
  }
  return filesys_remove (file);
}

int open_(const char* file)
{
  if (!file || !is_user_vaddr(file))
  {
    exit_(-1);
  }
  lock_acquire(&fd_mapping_list_lock); 
  struct file* f = filesys_open (file);
  
  if (!f) 
  {  
    lock_release(&fd_mapping_list_lock);
    return -1; // Returning -1 because some test cases do not like exit(-1)
  }

  pid_t pid = get_current_pid();
  int fd = get_and_increment_fd(pid);
  if(fd < 2)
  {
    lock_release(&fd_mapping_list_lock);
    exit_(-1);
  }

  struct fd_process *fd_process = malloc(sizeof(struct fd_process));
  fd_process->fd = fd;
  fd_process->pid = pid;
  fd_process->file = f;

  list_push_back (&fd_file_mapping, &fd_process->fd_elem);
  lock_release(&fd_mapping_list_lock);

  return fd;
}

int filesize_(int fd UNUSED)
{
  struct file* file = get_file(fd);
  if(!file)
  {
    return -1;
  }

  lock_acquire(&fd_mapping_list_lock);
  int size = file_length(file);
  lock_release(&fd_mapping_list_lock);

  return size;
}

int read_(int fd, void* buffer, unsigned size)
{
  if (!is_user_vaddr(buffer))
  {
    exit_(-1);
  }

  int bytes = -1;
  
  // Read from keyboard
  if(fd == STDIN_FILENO)
  {
    uint8_t* buf = (uint8_t*) buffer;
    for(bytes = 0; bytes < (int) size; bytes++)
    {
      buf[bytes] = input_getc();
    }
  }
  else
  {
    lock_acquire(&fd_mapping_list_lock);
    struct file* file = get_file(fd);
    if(file)
    {
      bytes = file_read(file, buffer, size);  
    }
    lock_release(&fd_mapping_list_lock);
  }

  return bytes;
}

int write(int fd, const void* buffer, unsigned size)
{
  // Writing to console
  if(fd == STDOUT_FILENO)
  {
    putbuf(buffer, size);
    return size;
  }

  struct file* file = get_file(fd);
  if(!file)
  {
    return -1;
  }

  lock_acquire(&fd_mapping_list_lock);
  int bytes = file_write(file, buffer, size);
	lock_release(&fd_mapping_list_lock);

  return bytes;
}

void seek_(int fd UNUSED, unsigned position UNUSED)
{
  // printf("seek called for fd = %d\n", fd);
	// TODO
  struct file *f;
  
  f = get_file(fd);
  if (!f)
  {
    return -1;
  }
  lock_acquire(&fd_mapping_list_lock);
  file_seek(f, position);
  lock_release(&fd_mapping_list_lock);
}

unsigned tell_(int fd UNUSED)
{
  // printf("tell called for fd = %d\n", fd);
	// TODO
  struct file *f;
  
  f = get_file(fd);
  if (!f)
  {
    return -1;
  }
  lock_acquire(&fd_mapping_list_lock);
  int tell = file_tell(f);
  lock_release(&fd_mapping_list_lock);
  return tell;
}

static void remove_fd(int fd)
{
  pid_t pid = get_current_pid();
  lock_acquire(&fd_mapping_list_lock);
  struct list_elem *e;
  struct list_elem *to_be_removed;
  for (e = list_begin (&fd_file_mapping); e != list_end (&fd_file_mapping); e = list_next (e))
  {
    struct fd_process *fd_process = list_entry (e, struct fd_process, fd_elem);
    if(fd_process->pid == pid && fd_process->fd == fd)
    {
      to_be_removed = e;
      //decrement fd counter
      fd_process->fd = fd -1;
      break;
    }
  }
  //remove fd from mapping list
  if (to_be_removed) {
    list_remove (to_be_removed);
  }

  lock_release(&fd_mapping_list_lock);
}

void close_(int fd UNUSED)
{
  // printf("close called for fd = %d\n", fd);
	// TODO
  struct file *f;
  
  f = get_file(fd);
  if (f)
  {
    file_close (f);
    remove_fd(fd);
  }
}
