.equ NR_ppoll, 309
.equ sizeof_sigset_t, 8

.text
.global ppoll

ppoll:
	xor	%eax, %eax
	mov	$NR_ppoll, %ax

	push	%edi
	push	%esi
	push	%ebx
	push	%ebp

	movl	%esp, %edi
	movl	5*4(%edi), %ebx
	movl	6*4(%edi), %ecx
	movl	7*4(%edi), %edx
	movl	8*4(%edi), %esi
	movl	$sizeof_sigset_t, %edi

	jmp	_syscallc

.type ppoll,@function
.size ppoll,.-ppoll
