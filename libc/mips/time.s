# auto-generated, remove this line before editing
.equ NR_time, 4013

.text
.set reorder
.global time
.ent time

time:
	li	$2, NR_time
	syscall
	la	$25, unisys
	jr	$25

.end time
