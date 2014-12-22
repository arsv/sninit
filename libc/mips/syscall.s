.text
.align 4
.globl syscall

.equ a0, 4
.equ a1, 5
.equ a2, 6
.equ a3, 7
.equ t8, 24
.equ t9, 25
.equ sp, 29

syscall:
	move	$v0, $a0	# syscall = a0
	move	$a0, $a1	# a0' = a1
	move	$a1, $a2	# a1' = a2
	move	$a2, $a3	# a2' = a3
	lw	$a3, 16($sp)	# a3' = a4
	lw	$t9, 20($sp)	# a4' = a5
	sw	$t9, 16($sp)
	lw	$t9, 24($sp)	# a5' = a6
	sw	$t9, 20($sp)
	syscall
	la	$t9, unisys
	jr	$t9

.type syscall,function
.size syscall,.-syscall
