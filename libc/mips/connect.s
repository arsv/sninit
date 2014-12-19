# auto-generated, remove this line before editing
.equ NR_connect, 4170

.text
.set reorder
.global connect
.ent connect

connect:
	li	$2, NR_connect
	syscall
	la	$25, unisys
	jr	$25

.end connect
