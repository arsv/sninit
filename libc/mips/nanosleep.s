# auto-generated, remove this line before editing
.equ NR_nanosleep, 4166

.text
.set reorder
.global nanosleep
.ent nanosleep

nanosleep:
	li	$2, NR_nanosleep
	syscall
	la	$25, unisys
	jr	$25

.end nanosleep
