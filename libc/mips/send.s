# auto-generated, remove this line before editing
.equ NR_send, 4178

.text
.set reorder
.global send
.ent send

send:
	li	$2, NR_send
	syscall
	la	$25, _syscall
	jr	$25

.end send
