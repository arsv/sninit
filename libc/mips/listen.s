# auto-generated, remove this line before editing
.equ NR_listen, 4174

.text
.set reorder
.global listen
.ent listen

listen:
	li	$2, NR_listen
	syscall
	la	$25, _syscall
	jr	$25

.end listen
