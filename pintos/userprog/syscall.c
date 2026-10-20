#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

/* 파일 시스템 전체를 한 번에 한 프로세스만 쓰게 막는 락. */
struct lock filesys_lock;

static void sys_halt (void);
static tid_t sys_fork (const char *thread_name, struct intr_frame *f);
static int sys_exec (const char *cmd_line);
static int sys_wait (tid_t pid);
static bool sys_create (const char *file, unsigned initial_size);
static bool sys_remove (const char *file);
static int sys_open (const char *file);
static int sys_filesize (int fd);
static int sys_read (int fd, void *buffer, unsigned size);
static int sys_write (int fd, const void *buffer, unsigned size);
static void sys_seek (int fd, unsigned position);
static unsigned sys_tell (int fd);
static void sys_close (int fd);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);

	/* 첫 유저 프로세스가 파일을 열기 전에 준비돼 있어야 한다. */
	lock_init (&filesys_lock);
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f) {
	// TODO: Your implementation goes here.
	/* 시스템 콜 번호는 rax, 인자는 rdi · rsi · rdx · r10 · r8 · r9 순서로 들어온다.
	   반환값이 있는 시스템 콜은 f->R.rax 에 써 넣으면 유저 쪽 함수의 반환값이 된다. */
	switch (f->R.rax) {
		case SYS_HALT:
			sys_halt ();
			break;
		case SYS_EXIT:
			sys_exit ((int) f->R.rdi);
			break;
		case SYS_FORK:
			f->R.rax = sys_fork ((const char *) f->R.rdi, f);
			break;
		case SYS_EXEC:
			f->R.rax = sys_exec ((const char *) f->R.rdi);
			break;
		case SYS_WAIT:
			f->R.rax = sys_wait ((tid_t) f->R.rdi);
			break;
		case SYS_CREATE:
			f->R.rax = sys_create ((const char *) f->R.rdi, (unsigned) f->R.rsi);
			break;
		case SYS_REMOVE:
			f->R.rax = sys_remove ((const char *) f->R.rdi);
			break;
		case SYS_OPEN:
			f->R.rax = sys_open ((const char *) f->R.rdi);
			break;
		case SYS_FILESIZE:
			f->R.rax = sys_filesize ((int) f->R.rdi);
			break;
		case SYS_READ:
			f->R.rax = sys_read ((int) f->R.rdi, (void *) f->R.rsi, (unsigned) f->R.rdx);
			break;
		case SYS_WRITE:
			f->R.rax = sys_write ((int) f->R.rdi, (const void *) f->R.rsi, (unsigned) f->R.rdx);
			break;
		case SYS_SEEK:
			sys_seek ((int) f->R.rdi, (unsigned) f->R.rsi);
			break;
		case SYS_TELL:
			f->R.rax = sys_tell ((int) f->R.rdi);
			break;
		case SYS_CLOSE:
			sys_close ((int) f->R.rdi);
			break;
		default:
			/* 모르는 번호면 유저 프로그램이 잘못 부른 것이므로 프로세스를 끝낸다. */
			sys_exit (-1);
	}
}

/* 핀토스를 끈다. (A) */
static void
sys_halt (void) {
	/* TODO (A): 구현. */
	sys_exit (-1);
}

/* 종료 상태를 남기고 프로세스를 끝낸다.
   종료 메시지는 process_exit () 에서 출력한다. (A) */
void
sys_exit (int status) {
	thread_current ()->exit_status = status;
	thread_exit ();
}

/* 현재 프로세스를 복제한다. 부모의 유저 레지스터가 담긴 f 를 자식에게 넘겨야 한다. (C) */
static tid_t
sys_fork (const char *thread_name UNUSED, struct intr_frame *f UNUSED) {
	/* TODO (C): 구현. */
	sys_exit (-1);
}

/* 현재 프로세스를 다른 프로그램으로 바꾼다. (A) */
static int
sys_exec (const char *cmd_line UNUSED) {
	/* TODO (A): 구현. */
	sys_exit (-1);
}

/* 자식이 끝날 때까지 기다리고 종료 상태를 받는다. (C) */
static int
sys_wait (tid_t pid) {
	return process_wait (pid);
}

/* 파일을 만든다. (B) */
static bool
sys_create (const char *file, unsigned initial_size) {
	bool success;

	/* 유저 포인터 검증은 A 담당 코드와 연동해야 한다. */
	validate_user_string (file);

	/* 파일 시스템에 접근하므로 락을 획득한다. */
	lock_acquire (&filesys_lock);

	/* 파일을 생성한다. 이미 존재하거나 생성할 수 없으면 false다. */
	success = filesys_create (file, initial_size);

	/* 성공과 실패 모두 락을 해제해야 한다. */
	lock_release (&filesys_lock);

	return success;
}

/* 파일을 지운다. (B) */
static bool
sys_remove (const char *file) {
	bool success;

	/* 파일 이름이 유저 메모리에 있는 문자열인지 검증한다. */
	validate_user_string (file);

	/* 파일 시스템에 접근하므로 락을 획득한다. */
	lock_acquire (&filesys_lock);

	/* 해당 파일을 삭제한다. */
	success = filesys_remove (file);

	/* 파일 시스템 락을 해제한다. */
	lock_release (&filesys_lock);

	return success;
}

/* 파일을 열고 fd 를 돌려준다. (B) */
static int
sys_open (const char *file) {
	struct file *opened_file;
	int fd;

	/* 파일 이름을 안전하게 읽을 수 있는지 검사한다. */
	validate_user_string (file);

	/* 파일 시스템 접근을 보호한다. */
	lock_acquire (&filesys_lock);

	/* 파일을 연다. 열기에 실패하면 NULL이다. */
	opened_file = filesys_open (file);

	if (opened_file == NULL) {
		lock_release (&filesys_lock);
		return -1;
	}

	/* 열린 파일을 현재 프로세스의 FD 테이블에 등록한다. */
	fd = fd_alloc (opened_file);

	/* FD를 할당하지 못했다면 열린 파일을 닫아 자원을 정리한다. */
	if (fd == -1) {
		file_close (opened_file);
		lock_release (&filesys_lock);
		return -1;
	}

	/* 모든 작업이 끝났으므로 락을 해제한다. */
	lock_release (&filesys_lock);

	return fd;
}

/* 파일 크기를 돌려준다. (B) */
static int
sys_filesize (int fd UNUSED) {
	/* TODO (B): 구현. */
	sys_exit (-1);
}

/* 파일 또는 키보드(fd 0)에서 읽는다. (B) */
static int
sys_read (int fd UNUSED, void *buffer UNUSED, unsigned size UNUSED) {
	/* TODO (B): 구현. */
	sys_exit (-1);
}

/* 콘솔(fd 1, A) 또는 파일(B)에 쓴다. */
static int
sys_write (int fd UNUSED, const void *buffer UNUSED, unsigned size UNUSED) {
	/* TODO (A): fd 1 콘솔. TODO (B): 파일. */
	sys_exit (-1);
}

/* 다음에 읽거나 쓸 위치를 옮긴다. (B) */
static void
sys_seek (int fd UNUSED, unsigned position UNUSED) {
	/* TODO (B): 구현. */
	sys_exit (-1);
}

/* 다음에 읽거나 쓸 위치를 돌려준다. (B) */
static unsigned
sys_tell (int fd UNUSED) {
	/* TODO (B): 구현. */
	sys_exit (-1);
}

/* fd 를 닫는다. (B) */
static void
sys_close (int fd) {
	struct file *file = fd_lookup (fd);

    /* 잘못된 FD이거나 이미 닫힌 FD면 무시한다. */
    if (file == NULL)
        return;

    /* 닫기 전에 슬롯을 비워 중복 close를 방지한다. */
    thread_current ()->fd_table[fd] = NULL;

    lock_acquire (&filesys_lock);
    file_close (file);
    lock_release (&filesys_lock);
}

/* 유저가 넘긴 주소 하나를 검사한다. NULL, 커널 영역, 매핑 안 된 주소면 exit (-1). (A) */
void
validate_user_addr (const void *uaddr UNUSED) {
	/* TODO (A): 구현. */
}

/* 버퍼 전체(buffer ~ buffer + size - 1)를 검사한다. 페이지 경계 포함. (A) */
void
validate_user_buffer (const void *buffer UNUSED, unsigned size UNUSED) {
	/* TODO (A): 구현. */
}

/* 문자열을 '\0' 까지 검사한다. (A) */
void
validate_user_string (const char *str UNUSED) {
	/* TODO (A): 구현. */
}

/* 빈 fd 칸에 파일을 넣고 그 번호를 돌려준다. 자리가 없으면 -1. (B) */
int
fd_alloc (struct file *file) {
	struct thread *cur = thread_current ();
	int fd;
	int checked = 0;

	/* 파일이 없거나 FD 테이블이 아직 준비되지 않았다면 할당하지 않는다. */
	if (file == NULL || cur->fd_table == NULL) {
		return -1;
	}

	/* 다음에 사용할 FD 번호부터 빈 슬롯 검색을 시작한다. */
	fd = cur->next_fd;

	/* FD 0과 1은 콘솔용으로 예약되어 있으므로 2부터 검색한다. */
	if (fd < 2 || fd >= FD_MAX) {
		fd = 2;
	}

	/* FD 테이블의 모든 파일 슬롯을 검사할 때까지 빈 번호를 찾는다. */
	while (checked < FD_MAX - 2) {
		/* 현재 FD 슬롯이 비어 있으면 해당 파일을 등록한다. */
		if (cur->fd_table[fd] == NULL) {
			cur->fd_table[fd] = file;

			/* 다음 할당은 이번 FD의 다음 번호부터 시작하도록 기록한다. */
			cur->next_fd = fd + 1;

			/* 테이블 끝에 도달하면 2부터 다시 검색한다. */
			if (cur->next_fd >= FD_MAX)
				cur->next_fd = 2;

			/* 파일이 등록된 FD 번호를 반환한다. */
			return fd;
		}

		/* 현재 슬롯이 사용 중이면 다음 FD 번호를 검사한다. */
		fd++;

		/* 테이블 범위를 벗어나면 2부터 다시 검색한다. */
		if (fd >= FD_MAX)
			fd = 2;

		checked++;
	}

	/* 사용 가능한 FD 슬롯이 없으면 할당 실패를 반환한다. */
	return -1;
}

/* fd 로 열린 파일을 찾는다. 없거나 범위 밖이면 NULL. (B) */
struct file *
fd_lookup (int fd) {
	struct thread *cur = thread_current ();

	/* FD 0과 1은 일반 파일용 FD가 아니다. */
	if (fd < 2 || fd >= FD_MAX)
		return NULL;

	/* FD 테이블이 아직 생성되지 않았다면 파일을 찾을 수 없다. */
	if (cur->fd_table == NULL)
		return NULL;

	/* 해당 슬롯의 파일 포인터를 반환한다. 빈 슬롯이면 NULL이다. */
	return cur->fd_table[fd];
}