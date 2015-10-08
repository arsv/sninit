# auto-generated, remove this line before editing
.equ NR_setresgid, 4190

.text
.set reorder
.global setresgid
.ent setresgid

setresgid:
	li	$2, NR_setresgid
	syscall
	la	$25, _syscall
	jr	$25

.end setresgid
