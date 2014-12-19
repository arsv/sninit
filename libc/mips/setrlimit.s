# auto-generated, remove this line before editing
.equ NR_setrlimit, 4075

.text
.set reorder
.global setrlimit
.ent setrlimit

setrlimit:
	li	$2, NR_setrlimit
	syscall
	la	$25, unisys
	jr	$25

.end setrlimit
