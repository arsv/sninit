.equ NR_exit, 60

.bss
.global environ

environ:	.quad 0

.type environ,@object
.size environ,.-environ


.text
.global _start
.global _exit

_start:
	movl	(%esp), %edi		/* %edi = argc */
	lea	4(%esp), %esi		/* %esi = argv */
	leaq	4(%rsi,%rdi,4),%rdx	/* %edx = envp = (4*edi)+%esi+4 */

	movl	%edx, environ(%rip)

	call	main

	movl	%eax, %edi		/* return value */

_exit:	mov	$NR_exit, %ax		/* call _exit */
	movzwl	%ax, %eax
	mov	%rcx, %r10
	syscall
	hlt				/* catch fire and die */

.type _exit,@function
.size _exit,.-_exit

.type _start,@function
.size _start,_exit-_start
