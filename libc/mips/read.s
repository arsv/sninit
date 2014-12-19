# auto-generated, remove this line before editing
.equ NR_read, 4003

.text
.set reorder
.global read
.ent read

read:
	li	$2, NR_read
	syscall
	la	$25, unisys
	jr	$25

.end read
