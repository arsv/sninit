.bss
.global errno

errno:	.int	0

.type errno,@object
.size errno,.-errno


.text
.global _syscall
.global _sysret
.global _syscallx

_syscall:
	mov	$0, %ah
_syscallx:
	movzwl	%ax, %eax
	mov	%rcx, %r10

	syscall
_sysret:			/* vfork jumps here */
	cmpq	$-132, %rax
	jbe	ok
	negl	%eax
	movl	%eax, errno
	orq	$-1, %rax
ok:	ret

.type _syscall,@function
.size _syscall,.-_syscall

.type _syscallx,@function
.size _syscallx,0
