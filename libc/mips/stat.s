# auto-generated, remove this line before editing
.equ NR_stat, 4106

.text
.set reorder
.global stat
.ent stat

stat:
	li	$2, NR_stat
	syscall
	la	$25, _syscall
	jr	$25

.end stat
