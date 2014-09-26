.bss
.global errno

errno:	.int	0

.type errno,@object
.size errno,.-errno


.data
.global vsyscall

vsyscall: .long sysint80h

.type vsyscall,@object
.size vsyscall,.-vsyscall


.text
.global unisys
.global unisysx

unisys:
	mov	$0, %ah
unisysx:
	movzwl	%ax, %eax

	push	%edi
	push	%esi
	push	%ebx
	push	%ebp

	movl	%esp, %edi
	movl	5*4(%edi), %ebx
	movl	6*4(%edi), %ecx
	movl	7*4(%edi), %edx
	movl	8*4(%edi), %esi
	movl   10*4(%edi), %ebp
	movl	9*4(%edi), %edi
	call	*vsyscall

	pop	%ebp
	pop	%ebx
	pop	%esi
	pop	%edi

	cmp	$-132,%eax
	jb	ok
	neg	%eax

	mov	%eax, errno
	sbb	%eax, %eax
ok:	ret

.type unisysx,@function
.size unisysx,.-unisysx

.type unisys,@function
.size unisys,unisysx-unisys

sysint80h:
	int	$0x80
	ret

.type sysint80h,@function
.size sysint80h,.-sysint80h
