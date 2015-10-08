# auto-generated, remove this line before editing
.equ NR_fstat, 4108

.text
.set reorder
.global fstat
.ent fstat

fstat:
	li	$2, NR_fstat
	syscall
	la	$25, _syscall
	jr	$25

.end fstat
