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

  return filesys_create (file, initial_size);
}

bool remove_(const char* file)
{
  // printf("remove called for filename = %s\n", file);
	// TODO
  if (!file || !is_user_vaddr(file))
  {
    exit_ (-1);
  }
  lock_acquire(&fd_mapping_list_lock);
  struct fd_process *matched_fd_mapping = malloc(sizeof(struct fd_process));
  struct list_elem *each_fd_mapping;
  bool result = false;
  
  //get the current thread id as there is 1:1 mapping between process and thread
  struct thread *cur_thread = thread_current ();
  matched_fd_mapping->pid = cur_thread->tid;
  matched_fd_mapping->fd = 0;
  bool mapping_found = false;
  for (each_fd_mapping = list_begin (&fd_file_mapping); each_fd_mapping != list_end (&fd_file_mapping);
       each_fd_mapping = list_next (each_fd_mapping))
  {
    //Find the highest fd mapping number for the current process.
    struct fd_process *fd_mapping_data = list_entry (each_fd_mapping, struct fd_process, fd_elem);
    if (fd_mapping_data->pid == matched_fd_mapping->pid && (matched_fd_mapping->fd < fd_mapping_data->fd)) {
      matched_fd_mapping = fd_mapping_data;//capture the current reference
      mapping_found = true;
      //break;
    }
  }
  if (!mapping_found) {
      //TODO handle this properly 
      exit_ (-1);
  }
  result = filesys_remove (file);
  if (!result) {
      lock_release(&fd_mapping_list_lock);
      exit_(-1);
  }
  list_remove (&matched_fd_mapping->fd_elem);
  lock_release(&fd_mapping_list_lock);
  return result;
}

int open_(const char* file)
{
  if (!file || !is_user_vaddr(file))
  {
    exit_(-1);
  }

  struct file* f = filesys_open (file);
  if (!f) 
  {  
    return -1; // Returning -1 because some test cases do not like exit(-1)
  }

  pid_t pid = get_current_pid();
  int fd = get_and_increment_fd(pid);
  if(fd < 2)
  {
    exit_(-1);
  }

  struct fd_process *fd_process = malloc(sizeof(struct fd_process));
  fd_process->fd = fd;
  fd_process->pid = pid;
  fd_process->file = f;

  lock_acquire(&fd_mapping_list_lock);  
  list_push_back (&fd_file_mapping, &fd_process->fd_elem);
  lock_release(&fd_mapping_list_lock);

  return fd;
}

int filesize_(int fd UNUSED)
{
  // printf("filesize called for fd = %d\n", fd);
	// TODO
  /*struct file *f;

  f = get_file(fd);
  if (!f)
  {
    return -1;
  }

  return file_length(f);*/
  return -1;
}

int read_(int fd UNUSED, void* buffer UNUSED, unsigned size UNUSED)
{
  // printf("read called for fd = %d\n", fd);
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

void seek_(int fd UNUSED, unsigned position UNUSED)
{
  // printf("seek called for fd = %d\n", fd);
	// TODO
  /*struct file *f;
  
  f = get_file(fd);
  if (!f)
  {
    return -1;
  }

  file_seek(f, pos);*/
}

unsigned tell_(int fd UNUSED)
{
  // printf("tell called for fd = %d\n", fd);
	// TODO
  /*struct file *f;
  
  f = get_file(fd);
  if (!f)
  {
    return -1;
  }

  return file_tell(f);*/
  return 0;
}

void close_(int fd UNUSED)
{
  // printf("close called for fd = %d\n", fd);
	// TODO
}
