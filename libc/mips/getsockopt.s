# auto-generated, remove this line before editing
.equ NR_getsockopt, 4173

.text
.set reorder
.global getsockopt
.ent getsockopt

getsockopt:
	li	$2, NR_getsockopt
	syscall
	la	$25, unisys
	jr	$25

.end getsockopt
