# auto-generated, remove this line before editing
.equ NR_mmap, 4090

.text
.set reorder
.global mmap
.ent mmap

mmap:
	li	$2, NR_mmap
	syscall
	la	$25, _syscall
	jr	$25

.end mmap
