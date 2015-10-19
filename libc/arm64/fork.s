/* NR_fork is listed as deprecated on newer arches including arm64 */
.equ NR_clone, 220
.equ SIGCHLD, 17

.text
.global fork

fork:
	mov	x0, SIGCHLD	/* flags */
	mov	x1, 0		/* child stack */
	mov	x2, 0		/* ptid */
	mov	x3, 0		/* pt_regs */
	mov	x4, 0		/* ctid */
	mov	x8, NR_clone
	b	_syscall

.type fork,function
.size fork,.-fork
