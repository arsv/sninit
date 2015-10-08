# auto-generated, remove this line before editing
.equ NR_waitpid, 4007

.text
.set reorder
.global waitpid
.ent waitpid

waitpid:
	li	$2, NR_waitpid
	syscall
	la	$25, _syscall
	jr	$25

.end waitpid
