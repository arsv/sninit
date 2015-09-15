.equ NR_sigprocmask, 126
.equ sizeof_sigset_t, 8

.text
.global sigprocmask

/* sigprocmask(how, set, old) := linux_sigprocmask(how, set, old, sizeof(sigset_t)) */
/*             4sp  8sp 12sp                       4sp  8sp  12sp    16sp */

/* To avoid messing with the stack to call unisys, we put a custom syscall preamble
   here and jump to the point in unisys where arguments are already in their respective
   registers. */

sigprocmask:
	xor	%eax, %eax
	mov	$NR_sigprocmask, %al

	push	%edi
	push	%esi
	push	%ebx
	push	%ebp

	movl	%esp, %edi
	movl	5*4(%edi), %ebx
	movl	6*4(%edi), %ecx
	movl	7*4(%edi), %edx
	movl	$sizeof_sigset_t, %esi

	jmp	unisysc

.type sigprocmask,@function
.size sigprocmask,.-sigprocmask
