# auto-generated, remove this line before editing
.equ NR_close, 4006

.text
.set reorder
.global close
.ent close

close:
	li	$2, NR_close
	syscall
	la	$25, _syscall
	jr	$25

.end close
