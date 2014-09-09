.text

.global unisys
.global unisysx
unisys:
	mov	$0, %ah
unisysx:
	movzwl	%ax, %eax
	mov	%rcx, %r10
	syscall
	cmpq	$-132, %rax
	jbe	ok
	negl	%eax
	pushq	%rax
	call	__errno_location
	popq	%rcx
	movl	%ecx,(%rax)
	orq	$-1, %rax
ok:	ret

.type unisysx,@function
.size unisysx,.-unisysx

.type unisys,@function
.size unisys,unisysx-unisys
