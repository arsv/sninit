# auto-generated, remove this line before editing
.equ NR_fcntl, 4055

.text
.set reorder
.global fcntl
.ent fcntl

fcntl:
	li	$2, NR_fcntl
	syscall
	la	$25, _syscall
	jr	$25

.end fcntl
