.equ NR_exit, 1

.data

environ: .word 0

.type environ,object
.size environ,.-environ


.text
.align 4

.global _start
.global _exit
.global environ

_start:

	mov	fp, #0			/* clear the frame pointer */
	ldr	a1, [sp]		/* argc */
	add	a2, sp, #4		/* argv */
	ldr	ip, =environ
	add	a3, a2, a1, lsl #2	/* &argv[argc] */
	add	a3, a3, #4		/* envp	*/
	str	a3, [ip, #0]		/* environ = envp */
	bl	main

_exit:
	mov	r7, #NR_exit
	swi	0			/* never returns */

.type _start,function
.size _start,_exit-_start

.type _exit,function
.size _exit,.-_exit
