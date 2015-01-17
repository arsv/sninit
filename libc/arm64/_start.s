.equ NR_exit, 93

.data

environ: .dword 0

.type environ,object
.size environ,.-environ


.text
.align 4

.global _start
.global _exit
.global environ

_start:

	mov	x30, #0			/* LR */
	mov	x29, sp			/* FP */

	ldr	x0, [sp]		/* argc */
	add	x1, sp, #8		/* argv */
	ldr	x3, =environ
	add	x2, x1, x0, lsl #3	/* &argv[argc] */
	add	x2, x2, #4		/* envp	 */
	str	x2, [x3, #0]		/* environ = envp */
	bl	main

_exit:
	mov	x8, #NR_exit
	svc	0
	hlt	0			/* halt and catch fire */

.type _start,function
.size _start,_exit-_start

.type _exit,function
.size _exit,.-_exit
