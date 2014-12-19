# auto-generated, remove this line before editing
.equ NR_getdents64, 4299

.text
.set reorder
.global getdents64
.ent getdents64

getdents64:
	li	$2, NR_getdents64
	syscall
	la	$25, unisys
	jr	$25

.end getdents64
