.text
.global syscall

syscall:
	mov	%rdi, %rax
	mov	%rsi, %rdi
	mov	%rdx, %rsi
	mov	%rcx, %rdx
	mov	%r8, %rcx	/* we're jumping to unisysx, which expects */
	mov	%r9, %r8	/* a function call with rcx instead of r10 */
	mov	0x8(%rsp), %r9
	jmp	unisysx

.type syscall,@function
.size syscall,.-syscall
