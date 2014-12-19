# auto-generated, remove this line before editing
.equ NR_openat, 4288

.text
.set reorder
.global openat
.ent openat

openat:
	li	$2, NR_openat
	syscall
	la	$25, unisys
	jr	$25

.end openat
