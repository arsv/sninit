.text
.global _start
_start:
	popq	%rdi			/* %rdi = argc */
	movq	%rsp,%rsi		/* %rsi = argv */
	pushq	%rdi
	
	leaq	8(%rsi,%rdi,8),%rdx	/* %rdx = envp = (8*rdi)+%rsi+8 */

	movq	%rdx, environ(%rip)

	call	main

	movq	%rax, %rdi	/* return value */
	call	exit

.Lstart:
	.size	 _start,.Lstart-_start

