# auto-generated, remove this line before editing
.equ NR_setitimer, 4104

.text
.set reorder
.global setitimer
.ent setitimer

setitimer:
	li	$2, NR_setitimer
	syscall
	la	$25, unisys
	jr	$25

.end setitimer
