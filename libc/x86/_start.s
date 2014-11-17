.equ NR_exit, 1
.equ AT_SYSINFO, 32

.bss
.global environ

environ:	.long 0

.type environ,@object
.size environ,.-environ


.text
.global _start
.global _exit

_start:
	popl	%ecx			/* %ecx = argc */
	movl	%esp, %eax		/* %eax = argv */
	pushl	%ecx
	/* Save environ for later use (which only happens in runcap, but we still need it to find auxv) */
	leal	4(%eax,%ecx,4),%esi	/* %esi = envp = (4*ecx)+%eax+4 */
	movl	%esi, environ

	/* Set up main arguments */
	pushl	%esi
	pushl	%eax
	pushl	%ecx

	/* Set up syscall trampoline pointer, which is in auxv under AT_SYSINFO key */
	/* auxv is an array of { long key, long val } pairs terminated by key 0 */
	/* See <elf.h>, getauxval(3), vdso(7) */

	/* Find auxv by skipping over envp (stored %esi by this point) */
1:	lodsl
	testl	%eax,%eax
	jnz	1b
	/* Iterate over auxv, looking for AT_SYSINFO key */
2:	lodsl				/* eax = key */
	or	%eax, %eax		/* if(!key) break */
	jz	3f
	cmpl	$AT_SYSINFO, %eax
	lodsl				/* eax = val */
	jne	2b
	movl	%eax, vsyscall		/* vsyscall = val */
3:
	call	main

	movl	%eax, %ebx		/* return value */
_exit:	xor	%eax, %eax
	movb	$NR_exit, %al		/* call _exit */
	call	*vsyscall
	hlt				/* catch fire and die */

.type _exit,@function
.size _exit,.-_exit

.type _start,@function
.size _start,_exit-_start
