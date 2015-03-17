.bss
.global errno

errno:	.int	0

.type errno,@object
.size errno,.-errno


.text
.global unisys
.global uniret
.global unisysx

unisys:
	mov	$0, %ah
unisysx:
	movzwl	%ax, %eax
	mov	%rcx, %r10

	syscall
uniret:				/* vfork jumps here */
	cmpq	$-132, %rax
	jbe	ok
	negl	%eax
	movl	%eax, errno
	orq	$-1, %rax
ok:	ret

.type unisys,@function
.size unisys,.-unisys

.type unisysx,@function
.size unisysx,0

.type uniret,@function
.size uniret,0
