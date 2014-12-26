.equ NR_ppoll, 4302
.equ sizeof_sigset_t, 16

.equ v0, 2
.equ t9, 25
.equ sp, 29

.text
.set reorder
.global ppoll
.ent ppoll

ppoll:
	li	$v0, NR_ppoll
	addi	$sp, $sp, -4
	li	$t9, sizeof_sigset_t
	sw	$t9, 0($sp)
	syscall
	addi	$sp, $sp, 4
	la	$t9, unisys
	jr	$t9

.end ppoll
