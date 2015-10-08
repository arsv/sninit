.text
.global sigaction

.equ NR_sigaction,	67
.equ sizeof_sigset_t,	8

/* sigaction(signum, act, oldact) := linux_sigaction(signum, act, oldact, sizeof(sigset_t)) */

sigaction:
	xor	%eax, %eax
	mov	$NR_sigaction, %al

	push	%edi
	push	%esi
	push	%ebx
	push	%ebp

	movl	%esp, %edi
	movl	5*4(%edi), %ebx
	movl	6*4(%edi), %ecx
	movl	7*4(%edi), %edx
	movl	$sizeof_sigset_t, %esi

	jmp	_syscallc

.type sigaction,@function
.size sigaction,.-sigaction
