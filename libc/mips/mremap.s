# auto-generated, remove this line before editing
.equ NR_mremap, 4167

.text
.set reorder
.global mremap
.ent mremap

mremap:
	li	$2, NR_mremap
	syscall
	la	$25, _syscall
	jr	$25

.end mremap
