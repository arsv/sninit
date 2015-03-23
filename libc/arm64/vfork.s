/* NR_vfork is listed as deprecated on newer arches including arm64 */
.equ NR_clone, 220
.equ SIGCHLD, 17
.equ CLONE_VM,    0x00000100
.equ CLONE_VFORK, 0x00004000

.text
.global vfork

vfork:
	mov	x0, (CLONE_VM | CLONE_VFORK | SIGCHLD)
	mov	x1, 0		/* child stack */
	mov	x2, 0		/* ptid */
	mov	x3, 0		/* ctid */
	mov	x4, 0		/* pt_regs */
	mov	x8, NR_clone
	b	unisys

.type vfork,function
.size vfork,.-vfork
