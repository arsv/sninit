.bss
.global errno

errno:	.int	0

.type errno,@object
.size errno,.-errno


.text
.global unisys
.global unisysx
.global uniret

unisys:
	mov	$0, %ah
unisysx:
	movzwl	%ax, %eax
	orl	$0x40000000, %eax
	mov	%rcx, %r10
	syscall
uniret:
	cmpq	$-132, %rax
	jbe	ok
	negl	%eax
	movl	%eax, errno
	orq	$-1, %rax
ok:	ret

.type unisysx,@function
.size unisysx,.-unisysx

.type unisys,@function
.size unisys,unisysx-unisys
