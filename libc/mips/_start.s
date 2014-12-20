/* Register names */
/* Now this results in rather weird disassembly (as in objdump -d), but it works */
.equ zero, 0
.equ ra, 31
.equ sp, 29
.equ a0, 4
.equ a1, 5
.equ a2, 6
.equ gp, 28

.equ NR_exit, 4001


.data
.align 4
.globl environ

environ: .word 0

.type environ,object
.size environ,.-environ


.text
.align 4
.global __start
.global _exit

__start:
	/* "All userspace code in Linux is PIC" -- http://www.linux-mips.org/wiki/PIC_code */
	.set noreorder
	bltzal $0,0f
	nop
0:	.cpload	$31

	.set reorder
	move	$ra, $zero	/* prime stack frame */
	lw	$a0, 0($sp)	/* load argc */
	addu	$a1, $sp, 4	/* load argv.  huh?  should be 4, right? */

	and	$sp, 0xfffffff8	/* align stack to 8 bytes */
	subu	$sp, 24		/* make room for 4 arguments, RA + pad */
	/* I don't understand the MIPS calling convention.  Why do you
	   need to make room on the stack for arguments you pass in
	   registers?  Anyway, if we don't do this, the arguments are
	   garbled. */
	sw	$ra, 20($sp)	/* close stack frame */

	addu	$a2, $a0, 1	/* load envp */
	sll	$a2, $a2, 2

	add	$a2, $a2, $a1
	sw	$a2, environ

	la	$25, main
	jalr	$25

_exit:
	li	$2,NR_exit
	syscall

.type __start,function
.size __start,_exit-__start

.type _exit,function
.size _exit,.-_exit
