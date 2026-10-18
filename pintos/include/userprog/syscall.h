#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <debug.h>
#include <stdbool.h>
#include "threads/synch.h"

struct file;

void syscall_init (void);

/* 파일 시스템은 내부 동기화가 없어서, filesys_* · file_* 호출은 전부 이 락 안에서 한다.
   process.c 의 load () 에서도 써야 하므로 여기서 공개한다. */
extern struct lock filesys_lock;

/* page_fault () 와 검증 실패에서도 프로세스를 끝내야 하므로 공개한다. */
void sys_exit (int status) NO_RETURN;

/* 유저 포인터 검증. 실패하면 exit (-1). 파일 시스템 락을 잡기 전에 불러야 한다. (A) */
void validate_user_addr (const void *uaddr);
void validate_user_buffer (const void *buffer, unsigned size);
void validate_user_string (const char *str);

/* fd 테이블 다루기. (B) */
int fd_alloc (struct file *file);
struct file *fd_lookup (int fd);

#endif /* userprog/syscall.h */