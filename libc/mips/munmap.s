# auto-generated, remove this line before editing
.equ NR_munmap, 4091

.text
.set reorder
.global munmap
.ent munmap

munmap:
	li	$2, NR_munmap
	syscall
	la	$25, _syscall
	jr	$25

.end munmap
