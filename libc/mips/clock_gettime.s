# auto-generated, remove this line before editing
.equ NR_clock_gettime, 4263

.text
.set reorder
.global clock_gettime
.ent clock_gettime

clock_gettime:
	li	$2, NR_clock_gettime
	syscall
	la	$25, _syscall
	jr	$25

.end clock_gettime
