# auto-generated, remove this line before editing
.equ NR_dup2, 4063

.text
.set reorder
.global dup2
.ent dup2

dup2:
	li	$2, NR_dup2
	syscall
	la	$25, _syscall
	jr	$25

.end dup2
