# auto-generated, remove this line before editing
.equ NR_chdir, 4012

.text
.set reorder
.global chdir
.ent chdir

chdir:
	li	$2, NR_chdir
	syscall
	la	$25, _syscall
	jr	$25

.end chdir
