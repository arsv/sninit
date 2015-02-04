# auto-generated, remove this line before editing
.equ NR_umask, 4060

.text
.set reorder
.global umask
.ent umask

umask:
	li	$2, NR_umask
	syscall
	la	$25, unisys
	jr	$25

.end umask
