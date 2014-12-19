# auto-generated, remove this line before editing
.equ NR_write, 4004

.text
.set reorder
.global write
.ent write

write:
	li	$2, NR_write
	syscall
	la	$25, unisys
	jr	$25

.end write
