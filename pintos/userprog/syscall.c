#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "threads/init.h" /* sys_halt 의 power_off () 를 쓰기 위해. */

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

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
}

/* 핀토스 전체를 끈다. 프로세스 종료가 아니므로 종료 메시지는 찍지 않는다. */
static void
sys_halt (void) {
	power_off ();
}

/* 버퍼의 SIZE 바이트를 FD 에 쓰고, 실제로 쓴 바이트 수를 반환한다.
   반환값은 syscall_handler 가 f->R.rax 에 넣어 유저에게 돌려준다. */
static int
sys_write (int fd, const void *buffer, unsigned size) {
	/* fd 1 은 콘솔. 여러 번 나눠 출력하면 다른 프로세스 출력과 섞일 수 있어
	   콘솔 락을 한 번만 잡는 putbuf () 로 한 번에 출력한다. */
	if (fd == 1) {
		putbuf (buffer, size);
		return size;
	}
	/* fd 0 과 파일 쓰기는 B (준우) 님이 채운다. */
	return -1;
}

/* 종료 상태를 남기고 끝낸다. 상태는 종료 메시지와 부모의 wait 가 읽는다. */
static void
sys_exit (int status) {
	thread_current ()->exit_status = status;
	thread_exit ();
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	printf ("system call!\n");
	thread_exit ();
}
