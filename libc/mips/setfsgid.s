# auto-generated, remove this line before editing
.equ NR_setfsgid, 4139

.text
.set reorder
.global setfsgid
.ent setfsgid

setfsgid:
	li	$2, NR_setfsgid
	syscall
	la	$25, _syscall
	jr	$25

.end setfsgid
