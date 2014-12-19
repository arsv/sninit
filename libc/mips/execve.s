# auto-generated, remove this line before editing
.equ NR_execve, 4011

.text
.set reorder
.global execve
.ent execve

execve:
	li	$2, NR_execve
	syscall
	la	$25, unisys
	jr	$25

.end execve
