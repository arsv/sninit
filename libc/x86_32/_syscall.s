.bss
.global errno

errno:	.int	0

.type errno,@object
.size errno,.-errno


.text
.global _syscall
.global _syscallx
.global _sysret

_syscall:
	mov	$0, %ah
_syscallx:
	movzwl	%ax, %eax
	orl	$0x40000000, %eax
	mov	%rcx, %r10
	syscall
_sysret:
	cmpq	$-132, %rax
	jbe	ok
	negl	%eax
	movl	%eax, errno
	orq	$-1, %rax
ok:	ret

.type _syscallx,@function
.size _syscallx,.-_syscallx

.type _syscall,@function
.size _syscall,_syscallx-_syscall
