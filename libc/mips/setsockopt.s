# auto-generated, remove this line before editing
.equ NR_setsockopt, 4181

.text
.set reorder
.global setsockopt
.ent setsockopt

setsockopt:
	li	$2, NR_setsockopt
	syscall
	la	$25, unisys
	jr	$25

.end setsockopt
