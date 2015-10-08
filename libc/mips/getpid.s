# auto-generated, remove this line before editing
.equ NR_getpid, 4020

.text
.set reorder
.global getpid
.ent getpid

getpid:
	li	$2, NR_getpid
	syscall
	la	$25, _syscall
	jr	$25

.end getpid
