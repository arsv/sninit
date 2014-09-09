#define __NR_exit 0x3c

.text
.global _start
.global _exit

_start:
	popq	%rdi			/* %rdi = argc */
	movq	%rsp,%rsi		/* %rsi = argv */
	pushq	%rdi
	
	leaq	8(%rsi,%rdi,8),%rdx	/* %rdx = envp = (8*rdi)+%rsi+8 */

	movq	%rdx, environ(%rip)

	call	main

	movq	%rax, %rdi		/* return value */

_exit:	mov	$__NR_exit, %ax		/* call _exit */
	movzwl	%ax, %eax
	mov	%rcx, %r10
	syscall
	jmp	0			/* catch fire and die */

.type _exit,@function
.size _exit,.-_exit

.type _start,@function
.size _start,_exit-_start
