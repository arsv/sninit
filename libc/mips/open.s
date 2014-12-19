# auto-generated, remove this line before editing
.equ NR_open, 4005

.text
.set reorder
.global open
.ent open

open:
	li	$2, NR_open
	syscall
	la	$25, unisys
	jr	$25

.end open
