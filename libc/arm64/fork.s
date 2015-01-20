/* NR_fork is listed as deprecated on newer arches including arm64 */
.equ NR_clone, 220

.text
.global fork

fork:
	mov	x0, 0		/* flags */
	mov	x1, 0		/* child stack */
	mov	x2, 0		/* ptid */
	mov	x3, 0		/* ctid */
	mov	x4, 0		/* pt_regs */
	mov	x8, NR_clone
	b	unisys

.type fork,function
.size fork,.-fork
