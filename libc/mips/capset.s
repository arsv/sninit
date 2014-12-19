# auto-generated, remove this line before editing
.equ NR_capset, 4205

.text
.set reorder
.global capset
.ent capset

capset:
	li	$2, NR_capset
	syscall
	la	$25, unisys
	jr	$25

.end capset
